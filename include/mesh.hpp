#pragma once

#include "obj_parser.hpp"

class Mesh {
public:
	Mesh() = default;
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	bool upload(const MeshData& data);
	void draw() const;
	const Vec3& center() const { return center_; }

private:
	unsigned int vao_ = 0;
	unsigned int vbo_ = 0;
	int vertexCount_ = 0;
	Vec3 center_{};
};
