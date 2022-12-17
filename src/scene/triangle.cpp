#include "triangle.hpp"
#include "../math/math.hpp"
#include <cmath>

Triangle::Triangle()
	: v0(0.0f), v1(0.0f), v2(0.0f), normal(0.0f),
	  uv0(0.0f), uv1(0.0f), uv2(0.0f),
	  edge1(0.0f), edge2(0.0f), mat_index(0)
{}

// NOTE: expects CCW winding order
Triangle::Triangle (Vec3f v0, Vec3f v1, Vec3f v2, uint32 mat_index)
	: v0(v0), v1(v1), v2(v2),
	  uv0(0), uv1(0), uv2(0),
	  mat_index(mat_index)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	normal = pixl::normalize(pixl::cross(A, B));
}

Triangle::Triangle (Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, uint32 mat_index)
	: v0(v0), v1(v1), v2(v2), normal(normal), mat_index(mat_index),
	  uv0(0.0f), uv1(0.0f), uv2(0.0f)
{}

Triangle::Triangle(Array<Vec3f> &vertices, Array<Vec3f> &normals, Array<Vec2f> &tex_coords, uint32 mat_index)
	: v0(vertices[0]), v1(vertices[1]), v2(vertices[2]),
	  uv0(tex_coords[0]), uv1(tex_coords[1]), uv2(tex_coords[2]),
	  mat_index(mat_index)
{
	normal = pixl::normalize(normals[0] + normals[1] + normals[2]);
}

float Area(Triangle *tri)
{
	return sqrtf(pixl::dot(tri->edge1, tri->edge1) * pixl::dot(tri->edge2, tri->edge2)) / 2.0f;
}

// TODO: replace with actual transformation matrices
void ApplyScaleToTriangle(Triangle *tri, Vec3f scale_vec)
{
	tri->v0.x *= scale_vec.x;
	tri->v1.x *= scale_vec.x;
	tri->v2.x *= scale_vec.x;

	tri->v0.y *= scale_vec.y;
	tri->v1.y *= scale_vec.y;
	tri->v2.y *= scale_vec.y;

	tri->v0.z *= scale_vec.z;
	tri->v1.z *= scale_vec.z;
	tri->v2.z *= scale_vec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}

// TODO: replace with actual transformation matrices
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translation_vec)
{
	tri->v0.x += translation_vec.x;
	tri->v1.x += translation_vec.x;
	tri->v2.x += translation_vec.x;

	tri->v0.y += translation_vec.y;
	tri->v1.y += translation_vec.y;
	tri->v2.y += translation_vec.y;

	tri->v0.z += translation_vec.z;
	tri->v1.z += translation_vec.z;
	tri->v2.z += translation_vec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}