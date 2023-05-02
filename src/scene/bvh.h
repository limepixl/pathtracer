#pragma once

#include "../defines.hpp"
#include "../core/array.hpp"
#include "triangle.hpp"

struct BVHNodeGLSL
{
    Vec4f data1; // bmin.x, bmin.y, bmin.z, left/first_tri
    Vec4f data2; // bmax.x, bmax.y, bmax.z, num_tris
};

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &triangles);