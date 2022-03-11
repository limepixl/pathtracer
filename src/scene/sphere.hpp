#pragma once
#include "ray.hpp"

struct Sphere
{
	Vec3f origin;
	float32 radius;
	
	struct Material *mat;
};

float32 Area(Sphere *sphere);
Sphere CreateSphere(Vec3f origin, float32 radius, struct Material *mat);
bool SphereIntersect(Ray ray, Sphere sphere, struct HitData *data, float32 &tmax);