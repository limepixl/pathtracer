#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

enum MaterialType
{
    MATERIAL_LAMBERTIAN = 0,
    MATERIAL_OREN_NAYAR,
    MATERIAL_SPECULAR_METAL
};

struct MaterialGLSL
{
    glm::vec4 data1 {}; // diff.x, diff.y, diff.z, roughness
	glm::vec4 data2 {}; // spec.x, spec.y, spec.z, mat_type
	glm::vec4 data3 {}; // Le.x, Le.y, Le.z, diffuse_tex_index

    MaterialGLSL();

    MaterialGLSL(const glm::vec3 &diffuse,
                 const glm::vec3 &specular,
                 const glm::vec3 &Le,
                 float roughness,
                 int diffuse_tex_index,
                 MaterialType type);

	[[nodiscard]] glm::vec3 diffuse() const;
	[[nodiscard]] glm::vec3 specular() const;
	[[nodiscard]] glm::vec3 emitted_radiance() const;
};
