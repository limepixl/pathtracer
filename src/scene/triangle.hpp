#pragma once
#include "../core/array.hpp"
#include "../math/vec.hpp"
#include "ray.hpp"

struct Triangle
{
  Vec3f v0, v1, v2;
  Vec3f normal;
  Vec2f uv0, uv1, uv2;

  // Precomputed values
  Vec3f edge1, edge2;

  uint32 mat_index;

  Triangle();

  Triangle(Vec3f v0, Vec3f v1, Vec3f v2, uint32 mat_index);

  Triangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, uint32 mat_index);

  Triangle(Array<Vec3f> &vertices, Array<Vec3f> &normals, Array<Vec2f> &tex_coords, uint32 mat_index);
};

float Area(Triangle *tri);

void ApplyScaleToTriangle(Triangle *tri, Vec3f scale_vec);

void ApplyTranslationToTriangle(Triangle *tri, Vec3f translation_vec);

struct TriangleGLSL
{
  Vec4f data1; // v0.x, v0.y, v0.z, mat_index
  Vec4f data2; // v1.x, v1.y, v1.z, 0
  Vec4f data3; // v2.x, v2.y, v2.z, 0
  Vec4f data4; // uv0.x, uv0.y, uv1.x, uv1.y
  Vec4f data5; // uv2.x, uv2.y, 0, 0
};
