#pragma once

#include "../defines.hpp"
#include "../core/array.hpp"
#include "triangle.hpp"

struct BVHNodeGLSL
{
    Vec4f data1; // bmin.x, bmin.y, bmin.z, left/first_tri
    Vec4f data2; // bmax.x, bmax.y, bmax.z, num_tris
    Vec4f data3; // axis, 0, 0, 0
};

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &triangles);