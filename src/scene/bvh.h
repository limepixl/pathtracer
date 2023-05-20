#pragma once

#include "../defines.hpp"
#include "../core/array.hpp"
#include "triangle.hpp"

struct BVHNodeGLSL
{
    glm::vec4 data1; // bmin.x, bmin.y, bmin.z, left/first_tri
	glm::vec4 data2; // bmax.x, bmax.y, bmax.z, num_tris

	BVHNodeGLSL() = default;
	BVHNodeGLSL(const glm::vec3 &bmin, const glm::vec3 &bmax, uint32 first_child_or_tri, uint32 num_tris);
};

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &glsl_tris, Array<TriangleGLSL> &sorted_glsl_tris);