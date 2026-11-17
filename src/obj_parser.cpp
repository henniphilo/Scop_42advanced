#include "obj_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {

struct FaceIdx {
	int v = -1;
	int vt = -1;
	int vn = -1;
};

float grayForFace(std::size_t faceIndex) {
	// Subtle gray shades in the spirit of the 42 logo.
	static const float shades[] = {
		0.35f, 0.42f, 0.48f, 0.55f, 0.62f, 0.70f, 0.78f, 0.40f, 0.58f, 0.66f};
	return shades[faceIndex % (sizeof(shades) / sizeof(shades[0]))];
}

FaceIdx parseIndex(const std::string& token, int vCount, int vtCount, int vnCount) {
	FaceIdx out;
	std::stringstream ss(token);
	std::string part;
	int slot = 0;
	while (std::getline(ss, part, '/')) {
		if (!part.empty()) {
			int idx = std::stoi(part);
			if (idx < 0) {
				if (slot == 0)
					idx = vCount + idx + 1;
				else if (slot == 1)
					idx = vtCount + idx + 1;
				else
					idx = vnCount + idx + 1;
			}
			if (slot == 0)
				out.v = idx - 1;
			else if (slot == 1)
				out.vt = idx - 1;
			else if (slot == 2)
				out.vn = idx - 1;
		}
		++slot;
	}
	return out;
}

Vec2 planarUv(const Vec3& p, const Vec3& minB, const Vec3& maxB) {
	const float dx = std::max(maxB.x - minB.x, 1e-6f);
	const float dy = std::max(maxB.y - minB.y, 1e-6f);
	const float dz = std::max(maxB.z - minB.z, 1e-6f);
	// Prefer the two largest axes for less stretching.
	if (dx <= dy && dx <= dz)
		return {(p.z - minB.z) / dz, (p.y - minB.y) / dy};
	if (dy <= dx && dy <= dz)
		return {(p.x - minB.x) / dx, (p.z - minB.z) / dz};
	return {(p.x - minB.x) / dx, (p.y - minB.y) / dy};
}

} // namespace

bool loadObj(const std::string& path, MeshData& out, std::string& error) {
	std::ifstream file(path);
	if (!file) {
		error = "Cannot open OBJ: " + path;
		return false;
	}

	std::vector<Vec3> positions;
	std::vector<Vec2> texcoords;
	std::vector<Vec3> normals;
	std::vector<std::vector<FaceIdx>> faces;

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		std::stringstream ss(line);
		std::string tag;
		ss >> tag;
		if (tag == "v") {
			Vec3 v;
			ss >> v.x >> v.y >> v.z;
			positions.push_back(v);
		} else if (tag == "vt") {
			Vec2 t;
			ss >> t.x >> t.y;
			texcoords.push_back(t);
		} else if (tag == "vn") {
			Vec3 n;
			ss >> n.x >> n.y >> n.z;
			normals.push_back(n);
		} else if (tag == "f") {
			std::vector<FaceIdx> face;
			std::string token;
			while (ss >> token) {
				face.push_back(parseIndex(
					token,
					static_cast<int>(positions.size()),
					static_cast<int>(texcoords.size()),
					static_cast<int>(normals.size())));
			}
			if (face.size() >= 3)
				faces.push_back(face);
		}
	}

	if (positions.empty() || faces.empty()) {
		error = "OBJ has no usable geometry: " + path;
		return false;
	}

	out.minBounds = positions[0];
	out.maxBounds = positions[0];
	for (const Vec3& p : positions) {
		out.minBounds.x = std::min(out.minBounds.x, p.x);
		out.minBounds.y = std::min(out.minBounds.y, p.y);
		out.minBounds.z = std::min(out.minBounds.z, p.z);
		out.maxBounds.x = std::max(out.maxBounds.x, p.x);
		out.maxBounds.y = std::max(out.maxBounds.y, p.y);
		out.maxBounds.z = std::max(out.maxBounds.z, p.z);
	}
	out.center = (out.minBounds + out.maxBounds) * 0.5f;

	out.vertices.clear();
	out.vertices.reserve(faces.size() * 3);

	for (std::size_t fi = 0; fi < faces.size(); ++fi) {
		const auto& face = faces[fi];
		const float g = grayForFace(fi);
		const Vec3 color{g, g, g};

		for (std::size_t i = 1; i + 1 < face.size(); ++i) {
			const FaceIdx tri[3] = {face[0], face[i], face[i + 1]};
			Vec3 triPos[3];
			bool haveNormal = true;
			Vec3 triNrm[3];
			for (int k = 0; k < 3; ++k) {
				if (tri[k].v < 0 || tri[k].v >= static_cast<int>(positions.size())) {
					error = "Invalid vertex index in " + path;
					return false;
				}
				triPos[k] = positions[static_cast<std::size_t>(tri[k].v)];
				if (tri[k].vn >= 0 && tri[k].vn < static_cast<int>(normals.size()))
					triNrm[k] = normals[static_cast<std::size_t>(tri[k].vn)];
				else
					haveNormal = false;
			}
			Vec3 faceNormal = normalize(cross(triPos[1] - triPos[0], triPos[2] - triPos[0]));
			if (length(faceNormal) < 1e-8f)
				faceNormal = {0.f, 1.f, 0.f};

			for (int k = 0; k < 3; ++k) {
				Vertex vert;
				vert.position = triPos[k] - out.center; // origin at object center
				vert.color = color;
				if (tri[k].vt >= 0 && tri[k].vt < static_cast<int>(texcoords.size()))
					vert.uv = texcoords[static_cast<std::size_t>(tri[k].vt)];
				else
					vert.uv = planarUv(triPos[k], out.minBounds, out.maxBounds);
				vert.normal = haveNormal ? normalize(triNrm[k]) : faceNormal;
				out.vertices.push_back(vert);
			}
		}
	}

	return true;
}
