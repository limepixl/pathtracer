#pragma once
#include "vec.hpp"

enum MaterialType
{
	MATERIAL_LAMBERTIAN = 0,
};

struct Material
{
	MaterialType type;
	Vec3f color;
	Vec3f Le; // emmision of light
};

Material CreateMaterial(MaterialType type, Vec3f color, Vec3f Le);