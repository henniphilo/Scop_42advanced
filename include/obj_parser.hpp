#pragma once

#include <string>
#include <vector>

#include "math.hpp"

struct Vertex {
	Vec3 position;
	Vec3 color;
	Vec2 uv;
	Vec3 normal;
};

struct MeshData {
	std::vector<Vertex> vertices;
	Vec3 center{};
	Vec3 minBounds{};
	Vec3 maxBounds{};
};

// Parses Wavefront .obj (v / vt / vn / f). Faces with >3 verts are fan-triangulated.
// Assigns subtle gray shades per face. Generates UVs if missing (planar from AABB).
bool loadObj(const std::string& path, MeshData& out, std::string& error);
