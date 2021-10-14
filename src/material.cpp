#include "material.hpp"
#include <string.h>

Material CreateMaterial(MaterialType type, Vec3f diffuse, Vec3f specular, Vec3f Le, const char *name)
{
	if(type == MaterialType::MATERIAL_LAMBERTIAN)
	{
		diffuse /= PI;
	}

	Material result = {type, diffuse, specular, Le, {}};

	size_t name_length = strlen(name);
	if(name_length > 0)
	{
		strncpy(result.name, name, 31);

		// Truncate string
		name_length = name_length <= 31 ? name_length : 31;
		result.name[name_length] = '\0';
	}

	return result;
}