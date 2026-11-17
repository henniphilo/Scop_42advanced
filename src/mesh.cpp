#include "mesh.hpp"

#include <cstddef>

#if defined(__APPLE__)
# include <OpenGL/gl3.h>
#else
# include <GL/gl.h>
#endif

Mesh::~Mesh() {
	if (vbo_ != 0)
		glDeleteBuffers(1, &vbo_);
	if (vao_ != 0)
		glDeleteVertexArrays(1, &vao_);
}

bool Mesh::upload(const MeshData& data) {
	if (data.vertices.empty())
		return false;

	center_ = data.center;
	vertexCount_ = static_cast<int>(data.vertices.size());

	if (vao_ == 0)
		glGenVertexArrays(1, &vao_);
	if (vbo_ == 0)
		glGenBuffers(1, &vbo_);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(data.vertices.size() * sizeof(Vertex)),
		data.vertices.data(),
		GL_STATIC_DRAW);

	const GLsizei stride = static_cast<GLsizei>(sizeof(Vertex));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, color)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, uv)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glBindVertexArray(0);
	return true;
}

void Mesh::draw() const {
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
	glBindVertexArray(0);
}
