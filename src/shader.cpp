#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#if defined(__APPLE__)
# include <OpenGL/gl3.h>
#else
# include <GL/gl.h>
#endif

Shader::~Shader() {
	if (program_ != 0)
		glDeleteProgram(program_);
}

std::string Shader::readFile(const std::string& path) {
	std::ifstream file(path);
	if (!file)
		throw std::runtime_error("Failed to open shader file: " + path);
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

unsigned int Shader::compile(unsigned int type, const std::string& source) {
	const unsigned int shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	int success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[2048];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		glDeleteShader(shader);
		throw std::runtime_error(std::string("Shader compile error: ") + log);
	}
	return shader;
}

bool Shader::link(unsigned int program) {
	glLinkProgram(program);
	int success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char log[2048];
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::cerr << "Shader link error: " << log << std::endl;
		return false;
	}
	return true;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
	try {
		const std::string vsSource = readFile(vertexPath);
		const std::string fsSource = readFile(fragmentPath);
		const unsigned int vs = compile(GL_VERTEX_SHADER, vsSource);
		const unsigned int fs = compile(GL_FRAGMENT_SHADER, fsSource);

		program_ = glCreateProgram();
		glAttachShader(program_, vs);
		glAttachShader(program_, fs);
		const bool ok = link(program_);
		glDeleteShader(vs);
		glDeleteShader(fs);
		if (!ok) {
			glDeleteProgram(program_);
			program_ = 0;
			return false;
		}
		return true;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}

void Shader::use() const { glUseProgram(program_); }

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(program_, name.c_str()), value ? 1 : 0);
}

void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(program_, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(program_, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const Vec3& value) const {
	glUniform3f(glGetUniformLocation(program_, name.c_str()), value.x, value.y, value.z);
}

void Shader::setMat4(const std::string& name, const Mat4& value) const {
	glUniformMatrix4fv(glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, value.data());
}
