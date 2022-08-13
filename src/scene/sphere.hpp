#pragma once
#include "ray.hpp"

struct Sphere
{
	Vec3f origin;
	float radius;

	uint32 mat_index;
};

float Area(Sphere *sphere);
Sphere CreateSphere(Vec3f origin, float radius, uint32 mat_index);
bool SphereIntersect(Ray ray, Sphere sphere, struct HitData *data, float &tmax);

struct SphereGLSL
{
	Vec4f data; // o.x, o.y, o.z, radius
	uint32 mat_index[4]; // mat_index, 0, 0, 0
};
