#pragma once
#include "../math/vec.hpp"

enum MaterialType
{
    MATERIAL_LAMBERTIAN = 0,
    MATERIAL_OREN_NAYAR,
    MATERIAL_SPECULAR_METAL
};

struct MaterialGLSL
{
    Vec4f data1 {}; // diff.x, diff.y, diff.z, roughness
    Vec4f data2 {}; // spec.x, spec.y, spec.z, mat_type
    Vec4f data3 {}; // Le.x, Le.y, Le.z, diffuse_tex_index

    MaterialGLSL();

    MaterialGLSL(const Vec3f &diffuse,
                 const Vec3f &specular,
                 const Vec3f &Le,
                 float roughness,
                 int diffuse_tex_index,
                 MaterialType type);

	[[nodiscard]] Vec3f diffuse() const;
	[[nodiscard]] Vec3f specular() const;
	[[nodiscard]] Vec3f emitted_radiance() const;
};
