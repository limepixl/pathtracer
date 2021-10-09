#include "material.hpp"
#include <string.h>

Material CreateMaterial(MaterialType type, Vec3f color, Vec3f Le, const char *name)
{
	if(type == MaterialType::MATERIAL_LAMBERTIAN)
	{
		color /= PI;
	}

	Material result = {type, color, Le, {}};

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