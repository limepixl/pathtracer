#pragma once
#include "../defines.hpp"
#include "../core/array.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct SphereGLSL
{
	glm::vec4 data {};       // o.x, o.y, o.z, radius
	glm::uvec4 mat_index {}; // mat_index, 0, 0, 0

    SphereGLSL() = default;

    SphereGLSL(glm::vec3 origin, float radius, uint32 mat_index);
};

Array<uint32> FindEmissiveSpheres(Array<SphereGLSL> &spheres, Array<struct MaterialGLSL> &materials);