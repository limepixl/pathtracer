#pragma once
#include "vec.hpp"
#include "defines.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

Vec3f PointAlongRay(Ray r, float32 t);