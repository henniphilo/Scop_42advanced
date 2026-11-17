#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#ifndef GLFW_INCLUDE_NONE
# define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#if defined(__APPLE__)
# include <OpenGL/gl3.h>
#else
# include <GL/gl.h>
#endif

#include "math.hpp"
#include "mesh.hpp"
#include "obj_parser.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace {

struct AppState {
	float rotX = 0.f;
	float rotY = 0.f;
	float rotZ = 0.f;
	float initialRotX = 0.f;
	float initialRotY = 0.f;
	float initialRotZ = 0.f;
	Vec3 translation{0.f, 0.f, 0.f};
	bool texturedTarget = false;
	float texMix = 0.f;
	bool autoSpin = true;
	bool wireframe = false;
	int width = 1280;
	int height = 720;

	void resetTransform() {
		rotX = initialRotX;
		rotY = initialRotY;
		rotZ = initialRotZ;
		translation = {0.f, 0.f, 0.f};
	}
};

AppState gState;

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
	gState.width = std::max(width, 1);
	gState.height = std::max(height, 1);
	glViewport(0, 0, gState.width, gState.height);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	const float step = 0.08f;
	const float rotStep = 0.08f;

	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_T:
		if (action == GLFW_PRESS)
			gState.texturedTarget = !gState.texturedTarget;
		break;
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
			gState.autoSpin = !gState.autoSpin;
		break;
	case GLFW_KEY_R:
		gState.resetTransform();
		break;
	case GLFW_KEY_Z:
		if (action == GLFW_PRESS) {
			gState.wireframe = !gState.wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, gState.wireframe ? GL_LINE : GL_FILL);
		}
		break;
	case GLFW_KEY_W:
		gState.translation.y += step;
		break;
	case GLFW_KEY_S:
		gState.translation.y -= step;
		break;
	case GLFW_KEY_A:
		gState.translation.x -= step;
		break;
	case GLFW_KEY_D:
		gState.translation.x += step;
		break;
	case GLFW_KEY_Q:
		gState.translation.z += step;
		break;
	case GLFW_KEY_E:
		gState.translation.z -= step;
		break;
	case GLFW_KEY_UP:
		gState.rotX -= rotStep;
		break;
	case GLFW_KEY_DOWN:
		gState.rotX += rotStep;
		break;
	case GLFW_KEY_LEFT:
		gState.rotY -= rotStep;
		break;
	case GLFW_KEY_RIGHT:
		gState.rotY += rotStep;
		break;
	case GLFW_KEY_J:
		gState.rotZ -= rotStep;
		break;
	case GLFW_KEY_L:
		gState.rotZ += rotStep;
		break;
	default:
		break;
	}
}

void printHelp(const char* exe) {
	std::cout
		<< "Usage: " << exe << " [path/to/model.obj]\n"
		<< "Controls:\n"
		<< "  Arrows / J L   rotate around X / Y / Z\n"
		<< "  WASD Q E       translate\n"
		<< "  T              toggle texture (smooth blend)\n"
		<< "  Space          toggle auto-spin\n"
		<< "  R              reset transform\n"
		<< "  Z              wireframe\n"
		<< "  Esc            quit\n";
}

} // namespace

int main(int argc, char** argv) {
	printHelp(argv[0]);

	const std::string objPath = (argc > 1) ? argv[1] : "resources/42.obj";
	const std::string texPath = "resources/kittens.bmp";

	if (!glfwInit()) {
		std::cerr << "Failed to init GLFW\n";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(gState.width, gState.height, "scop", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	int fbw = 0;
	int fbh = 0;
	glfwGetFramebufferSize(window, &fbw, &fbh);
	framebufferSizeCallback(window, fbw, fbh);

	glEnable(GL_DEPTH_TEST);
	// OBJ winding is often inconsistent (42 logo / teapot) — culling would drop faces.
	glDisable(GL_CULL_FACE);
	glClearColor(0.12f, 0.12f, 0.14f, 1.f);

	MeshData meshData;
	std::string error;
	if (!loadObj(objPath, meshData, error)) {
		std::cerr << error << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	Mesh mesh;
	if (!mesh.upload(meshData)) {
		std::cerr << "Failed to upload mesh\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	Shader shader;
	if (!shader.loadFromFiles("shaders/vertex.glsl", "shaders/fragment.glsl")) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	Texture texture;
	if (!texture.loadBMP(texPath)) {
		std::cerr << "Texture load failed, continuing with colors only\n";
	}

	const float extent = length(meshData.maxBounds - meshData.minBounds);
	const float camDist = std::max(extent * 1.6f, 3.f);

	// Only auto-orient clearly flat models (e.g. 42 logo). Teapot is shortest on Y
	// but not flat — tipping it 90° would stand it on the spout.
	{
		const Vec3 size = meshData.maxBounds - meshData.minBounds;
		float axes[3] = {size.x, size.y, size.z};
		std::sort(axes, axes + 3);
		const bool flat = axes[1] > 1e-6f && (axes[0] / axes[1]) < 0.35f;
		if (flat) {
			const float pi2 = 1.57079632679f;
			if (size.x <= size.y && size.x <= size.z)
				gState.initialRotY = -pi2;
			else if (size.y <= size.x && size.y <= size.z)
				gState.initialRotX = pi2;
		}
		gState.resetTransform();
	}

	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		const double now = glfwGetTime();
		const float dt = static_cast<float>(now - lastTime);
		lastTime = now;

		glfwPollEvents();

		if (gState.autoSpin)
			gState.rotY += dt * 0.7f;

		const float target = gState.texturedTarget ? 1.f : 0.f;
		const float speed = 2.5f;
		if (gState.texMix < target)
			gState.texMix = std::min(target, gState.texMix + dt * speed);
		else if (gState.texMix > target)
			gState.texMix = std::max(target, gState.texMix - dt * speed);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const float aspect = static_cast<float>(gState.width) / static_cast<float>(gState.height);
		const Mat4 proj = Mat4::perspective(60.f * 3.14159265f / 180.f, aspect, 0.1f, 200.f);
		const Mat4 view = Mat4::lookAt({0.f, 0.f, camDist}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});

		// Rotate/translate around object center (mesh vertices already centered).
		const Mat4 model =
			Mat4::translation(gState.translation) *
			Mat4::rotationY(gState.rotY) *
			Mat4::rotationX(gState.rotX) *
			Mat4::rotationZ(gState.rotZ);

		shader.use();
		shader.setMat4("uModel", model);
		shader.setMat4("uView", view);
		shader.setMat4("uProj", proj);
		shader.setFloat("uTexMix", gState.texMix);
		shader.setVec3("uLightDir", normalize(Vec3{0.4f, -1.0f, -0.3f}));
		shader.setVec3("uCameraPos", Vec3{0.f, 0.f, camDist});
		shader.setInt("uTexture", 0);
		if (texture.valid())
			texture.bind(0);

		mesh.draw();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
