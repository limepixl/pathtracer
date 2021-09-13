#pragma once

struct Material
{
	Vec3f color;
	Vec3f Le; // emmision of light
};

Material CreateMaterial(Vec3f color, Vec3f Le)
{
	return {color, Le};
}