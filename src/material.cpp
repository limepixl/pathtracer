#include "material.hpp"

Material CreateMaterial(MaterialType type, Vec3f color, Vec3f Le)
{
	if(type == MaterialType::MATERIAL_LAMBERTIAN)
		color /= PI;

	return {type, color, Le};
}