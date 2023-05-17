#pragma once
#include "../core/array.hpp"
#include "../math/vec.hpp"
#include "ray.hpp"

struct Triangle
{
    Vec3f v0, v1, v2;
    Vec3f n0, n1, n2;
    Vec2f uv0, uv1, uv2;

    // Precomputed values
    Vec3f edge1, edge2;

    uint32 mat_index;

    Triangle();

    Triangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f n0, Vec3f n1, Vec3f n2, uint32 mat_index);

    Triangle(Array<Vec3f> &vertices, Array<Vec3f> &normals, Array<Vec2f> &tex_coords, uint32 mat_index);
};

struct TriangleGLSL
{
    Vec4f data1; // v0.x, v0.y, v0.z, mat_index
    Vec4f data2; // v1.x, v1.y, v1.z, uv0.x
    Vec4f data3; // v2.x, v2.y, v2.z, uv0.y
    Vec4f data4; // uv1.x, uv1.y, uv2.x, uv2.y
	Vec4f data5; // n0.x, n0.y, n0.z, n1.x
	Vec4f data6; // n1.y, n1.z, n2.x, n2.y
	Vec4f data7; // n2.z, 0, 0, 0

	TriangleGLSL() = default;
	explicit TriangleGLSL(const Triangle &triangle);
	TriangleGLSL(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2,
				 const Vec2f &uv0, const Vec2f &uv1, const Vec2f &uv2,
				 const Vec3f &n0, const Vec3f &n1, const Vec3f &n2,
				 uint32 mat_index);

	[[nodiscard]] Vec3f v0() const;
	[[nodiscard]] Vec3f v1() const;
	[[nodiscard]] Vec3f v2() const;
};
