#pragma once
#include "../math/vec.hpp"

struct SphereGLSL
{
	Vec4f data; // o.x, o.y, o.z, radius
	uint32 mat_index[4]; // mat_index, 0, 0, 0
};
