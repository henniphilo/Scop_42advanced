#include "math.hpp"

Mat4 Mat4::identity() {
	Mat4 r;
	r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
	return r;
}

Mat4 Mat4::translation(const Vec3& t) {
	Mat4 r = identity();
	r.m[12] = t.x;
	r.m[13] = t.y;
	r.m[14] = t.z;
	return r;
}

Mat4 Mat4::scale(const Vec3& s) {
	Mat4 r = identity();
	r.m[0] = s.x;
	r.m[5] = s.y;
	r.m[10] = s.z;
	return r;
}

Mat4 Mat4::rotationX(float radians) {
	Mat4 r = identity();
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	r.m[5] = c;
	r.m[6] = s;
	r.m[9] = -s;
	r.m[10] = c;
	return r;
}

Mat4 Mat4::rotationY(float radians) {
	Mat4 r = identity();
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	r.m[0] = c;
	r.m[2] = -s;
	r.m[8] = s;
	r.m[10] = c;
	return r;
}

Mat4 Mat4::rotationZ(float radians) {
	Mat4 r = identity();
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	r.m[0] = c;
	r.m[1] = s;
	r.m[4] = -s;
	r.m[5] = c;
	return r;
}

Mat4 Mat4::perspective(float fovYRadians, float aspect, float nearPlane, float farPlane) {
	Mat4 r;
	const float f = 1.f / std::tan(fovYRadians * 0.5f);
	r.m[0] = f / aspect;
	r.m[5] = f;
	r.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
	r.m[11] = -1.f;
	r.m[14] = (2.f * farPlane * nearPlane) / (nearPlane - farPlane);
	return r;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
	const Vec3 f = normalize(center - eye);
	const Vec3 s = normalize(cross(f, up));
	const Vec3 u = cross(s, f);

	Mat4 r = identity();
	r.m[0] = s.x;
	r.m[4] = s.y;
	r.m[8] = s.z;
	r.m[1] = u.x;
	r.m[5] = u.y;
	r.m[9] = u.z;
	r.m[2] = -f.x;
	r.m[6] = -f.y;
	r.m[10] = -f.z;
	r.m[12] = -dot(s, eye);
	r.m[13] = -dot(u, eye);
	r.m[14] = dot(f, eye);
	return r;
}

Mat4 Mat4::operator*(const Mat4& other) const {
	Mat4 r;
	for (int col = 0; col < 4; ++col) {
		for (int row = 0; row < 4; ++row) {
			r.m[col * 4 + row] =
				m[0 * 4 + row] * other.m[col * 4 + 0] +
				m[1 * 4 + row] * other.m[col * 4 + 1] +
				m[2 * 4 + row] * other.m[col * 4 + 2] +
				m[3 * 4 + row] * other.m[col * 4 + 3];
		}
	}
	return r;
}
