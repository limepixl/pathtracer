#pragma once
#include "../math/vec.hpp"

struct SphereGLSL
{
  Vec4f data; // o.x, o.y, o.z, radius
  uint32 mat_index[4]; // mat_index, 0, 0, 0

  SphereGLSL() = default;

  SphereGLSL(Vec3f origin, float radius, uint32 mat_index)
      : data(Vec4f(origin.x, origin.y, origin.z, radius))
  {
    this->mat_index[0] = mat_index;
  }
};
