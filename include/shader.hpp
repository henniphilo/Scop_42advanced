#pragma once

#include <string>

#include "math.hpp"

class Shader {
public:
	Shader() = default;
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
	void use() const;

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, const Vec3& value) const;
	void setMat4(const std::string& name, const Mat4& value) const;

	unsigned int id() const { return program_; }

private:
	unsigned int program_ = 0;

	static std::string readFile(const std::string& path);
	static unsigned int compile(unsigned int type, const std::string& source);
	static bool link(unsigned int program);
};
