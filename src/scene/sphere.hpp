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