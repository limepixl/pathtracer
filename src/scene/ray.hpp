#pragma once
#include "../defines.hpp"
#include "../math/vec.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
	Vec3f invDir;
};

Vec3f PointAlongRay(Ray r, float t);