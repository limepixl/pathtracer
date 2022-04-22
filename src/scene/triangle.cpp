#include "triangle.hpp"
#include "../math/math.hpp"
#include "scene.hpp"
#include <cmath>

float Area(Triangle *tri)
{
	return sqrtf(Dot(tri->edge1, tri->edge1) * Dot(tri->edge2, tri->edge2)) / 2.0f;
}

// NOTE: expects CCW winding order
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Material *mat)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	Vec3f normal = NormalizeVec3f(Cross(A, B));
	return { v0, v1, v2, normal, A, B, mat };
}

Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, Material *mat)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	return { v0, v1, v2, normal, A, B, mat };
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
		data->mat = tri.mat;
		data->objectType = ObjectType::TRIANGLE;
		return true;
	}

	return false;
}

// TODO: replace with actual transformation matrices
void ApplyScaleToTriangle(Triangle *tri, Vec3f scaleVec)
{
	tri->v0.x *= scaleVec.x;
	tri->v1.x *= scaleVec.x;
	tri->v2.x *= scaleVec.x;

	tri->v0.y *= scaleVec.y;
	tri->v1.y *= scaleVec.y;
	tri->v2.y *= scaleVec.y;

	tri->v0.z *= scaleVec.z;
	tri->v1.z *= scaleVec.z;
	tri->v2.z *= scaleVec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}

// TODO: replace with actual transformation matrices
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translationVec)
{
	tri->v0.x += translationVec.x;
	tri->v1.x += translationVec.x;
	tri->v2.x += translationVec.x;

	tri->v0.y += translationVec.y;
	tri->v1.y += translationVec.y;
	tri->v2.y += translationVec.y;

	tri->v0.z += translationVec.z;
	tri->v1.z += translationVec.z;
	tri->v2.z += translationVec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}
