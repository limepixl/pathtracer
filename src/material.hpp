#pragma once
#include "vec.hpp"

enum MaterialType
{
	MATERIAL_LAMBERTIAN = 0,
	MATERIAL_IDEAL_REFLECTIVE = 1,
};

struct Material
{
	MaterialType type;
	Vec3f diffuse;
	Vec3f specular;
	Vec3f Le; // emmision of light

	// Optional
	char name[32];
};

Material CreateMaterial(MaterialType type, Vec3f diffuse, Vec3f specular, Vec3f Le, const char *name = "");