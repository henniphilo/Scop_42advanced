#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vColor;
out vec2 vUV;
out vec3 vNormal;
out vec3 vWorldPos;

void main() {
	vec4 world = uModel * vec4(aPos, 1.0);
	vWorldPos = world.xyz;
	vColor = aColor;
	vUV = aUV;
	vNormal = mat3(uModel) * aNormal;
	gl_Position = uProj * uView * world;
}
