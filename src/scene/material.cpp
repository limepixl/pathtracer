#include "material.hpp"
#include <cstring>

Material CreateMaterial(MaterialType type, Vec3f diffuse, Vec3f specular, float n_spec, Vec3f Le, const char *name)
{
	if (type == MaterialType::MATERIAL_LAMBERTIAN || type == MaterialType::MATERIAL_PHONG || type == MaterialType::MATERIAL_BLINN_PHONG)
	{
		diffuse /= PI;
	}

	Material result = { type, diffuse, specular, n_spec, Le, {} };

	size_t name_length = strlen(name);
	if (name_length > 0)
	{
		strncpy(result.name, name, 31);

		// Truncate string
		name_length = name_length <= 31 ? name_length : 31;
		result.name[name_length] = '\0';
	}

	return result;
}
