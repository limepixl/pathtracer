#pragma once

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

Material CreateMaterial(MaterialType type, Vec3f color, Vec3f Le)
{
	if(type == MaterialType::MATERIAL_LAMBERTIAN)
		color /= PI;

	return {type, color, Le};
}