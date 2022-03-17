#pragma once
#include "../math/vec.hpp"
#include "../defines.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
	Vec3f invDir;
};

Vec3f PointAlongRay(Ray r, float t);