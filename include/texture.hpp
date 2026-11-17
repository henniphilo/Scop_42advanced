#pragma once

#include <string>

class Texture {
public:
	Texture() = default;
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	bool loadBMP(const std::string& path);
	void bind(unsigned int unit = 0) const;
	bool valid() const { return id_ != 0; }

private:
	unsigned int id_ = 0;
	int width_ = 0;
	int height_ = 0;
};
