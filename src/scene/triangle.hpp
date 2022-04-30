#pragma once
#include "../math/vec.hpp"
#include "ray.hpp"

struct Triangle
{
	Vec3f v0, v1, v2;
	Vec3f normal;

	// Precomputed values
	Vec3f edge1, edge2;

	struct Material *mat;
};

float Area(Triangle *tri);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, struct Material *mat);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, struct Material *mat);
bool TriangleIntersect(Ray ray, Triangle &tri, struct HitData *data, float tmax);
void ApplyScaleToTriangle(Triangle *tri, Vec3f scale_vec);
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translation_vec);