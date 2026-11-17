#pragma once

#include <cmath>
#include <cstddef>

struct Vec2 {
	float x = 0.f;
	float y = 0.f;
};

struct Vec3 {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	Vec3() = default;
	Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
	Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
	Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
	Vec3& operator+=(const Vec3& o) {
		x += o.x;
		y += o.y;
		z += o.z;
		return *this;
	}
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

inline float dot(const Vec3& a, const Vec3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
	return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(const Vec3& v) {
	const float len = length(v);
	if (len < 1e-8f)
		return {0.f, 0.f, 0.f};
	return v * (1.f / len);
}

// Column-major 4x4 matrix (OpenGL layout).
struct Mat4 {
	float m[16]{};

	static Mat4 identity();
	static Mat4 translation(const Vec3& t);
	static Mat4 scale(const Vec3& s);
	static Mat4 rotationX(float radians);
	static Mat4 rotationY(float radians);
	static Mat4 rotationZ(float radians);
	static Mat4 perspective(float fovYRadians, float aspect, float nearPlane, float farPlane);
	static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

	Mat4 operator*(const Mat4& other) const;
	const float* data() const { return m; }
};
