#include "triangle.hpp"
#include "../math/math.hpp"
#include "scene.hpp"
#include <cmath>

float Area(Triangle *tri)
{
	return sqrtf(Dot(tri->edge1, tri->edge1) * Dot(tri->edge2, tri->edge2)) / 2.0f;
}

// NOTE: expects CCW winding order
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, uint32 mat_index)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	Vec3f normal = NormalizeVec3f(Cross(A, B));
	return { v0, v1, v2, normal, A, B, mat_index };
}

Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, uint32 mat_index)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	return { v0, v1, v2, normal, A, B, mat_index };
}

// Moller–Trumbore ray-triangle intersection algorithm
bool TriangleIntersect(Ray ray, Triangle &tri, HitData *data, float tmax)
{
	Vec3f pvec = Cross(ray.direction, tri.edge2);
	float determinant = Dot(tri.edge1, pvec);

	// Ray direction parallel to the triangle plane
#if TWO_SIDED_LIGHT
	if (Abs(determinant) < EPSILON)
		return false;
#else
	if (determinant < EPSILON)
		return false;
#endif

	float inv_determinant = 1.0f / determinant;
	Vec3f tvec = ray.origin - tri.v0;
	float u = Dot(tvec, pvec) * inv_determinant;
	if ((u < 0.0f) || (u > 1.0f))
		return false;

	Vec3f qvec = Cross(tvec, tri.edge1);
	float v = inv_determinant * Dot(ray.direction, qvec);
	if ((v < 0.0f) || (u + v > 1.0f))
		return false;

	// Computing t
	float t = inv_determinant * Dot(tri.edge2, qvec);
	if (t > TMIN && t < tmax)
	{
		data->t = t;
		data->point = ray.origin + ray.direction * t;
		data->normal = tri.normal;
		data->mat_index = tri.mat_index;
		data->object_type = ObjectType::TRIANGLE;
		return true;
	}

	return false;
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
