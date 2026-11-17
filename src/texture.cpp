#include "texture.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#if defined(__APPLE__)
# include <OpenGL/gl3.h>
#else
# include <GL/gl.h>
#endif

namespace {

#pragma pack(push, 1)
struct BmpFileHeader {
	uint16_t type;
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
};

struct BmpInfoHeader {
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t imageSize;
	int32_t xPelsPerMeter;
	int32_t yPelsPerMeter;
	uint32_t clrUsed;
	uint32_t clrImportant;
};
#pragma pack(pop)

} // namespace

Texture::~Texture() {
	if (id_ != 0)
		glDeleteTextures(1, &id_);
}

bool Texture::loadBMP(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open BMP: " << path << std::endl;
		return false;
	}

	BmpFileHeader fileHeader{};
	BmpInfoHeader infoHeader{};
	file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

	if (!file || fileHeader.type != 0x4D42) {
		std::cerr << "Invalid BMP header: " << path << std::endl;
		return false;
	}
	if (infoHeader.bitCount != 24 || infoHeader.compression != 0) {
		std::cerr << "Only uncompressed 24-bit BMP supported: " << path << std::endl;
		return false;
	}

	width_ = infoHeader.width;
	height_ = std::abs(infoHeader.height);
	const bool topDown = infoHeader.height < 0;
	const int rowSize = ((width_ * 3 + 3) / 4) * 4;
	std::vector<unsigned char> fileData(static_cast<size_t>(rowSize) * height_);
	file.seekg(fileHeader.offset, std::ios::beg);
	file.read(reinterpret_cast<char*>(fileData.data()), static_cast<std::streamsize>(fileData.size()));
	if (!file) {
		std::cerr << "Failed to read BMP pixels: " << path << std::endl;
		return false;
	}

	std::vector<unsigned char> rgba(static_cast<size_t>(width_ * height_ * 4));
	for (int y = 0; y < height_; ++y) {
		const int srcY = topDown ? y : (height_ - 1 - y);
		const unsigned char* row = fileData.data() + static_cast<size_t>(srcY) * rowSize;
		for (int x = 0; x < width_; ++x) {
			const size_t dst = static_cast<size_t>((y * width_ + x) * 4);
			rgba[dst + 0] = row[x * 3 + 2];
			rgba[dst + 1] = row[x * 3 + 1];
			rgba[dst + 2] = row[x * 3 + 0];
			rgba[dst + 3] = 255;
		}
	}

	if (id_ != 0)
		glDeleteTextures(1, &id_);
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture::bind(unsigned int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id_);
}
