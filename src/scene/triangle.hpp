#pragma once
#include "../math/vec.hpp"
#include "ray.hpp"

struct Triangle
{
	Vec3f v0, v1, v2;
	Vec3f normal;

	// Precomputed values
	Vec3f edge1, edge2;

	uint32 mat_index;
};

float Area(Triangle *tri);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, uint32 mat_index);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, uint32 mat_index);
bool TriangleIntersect(Ray ray, Triangle &tri, struct HitData *data, float tmax);
void ApplyScaleToTriangle(Triangle *tri, Vec3f scale_vec);
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translation_vec);
