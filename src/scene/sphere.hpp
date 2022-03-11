#pragma once
#include "ray.hpp"

struct Sphere
{
	Vec3f origin;
	float radius;
	
	struct Material *mat;
};

float Area(Sphere *sphere);
Sphere CreateSphere(Vec3f origin, float radius, struct Material *mat);
bool SphereIntersect(Ray ray, Sphere sphere, struct HitData *data, float &tmax);