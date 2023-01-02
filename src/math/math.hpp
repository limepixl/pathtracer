#pragma once
#include "../../thirdparty/pcg-c-basic-0.9/pcg_basic.h"
#include "../defines.hpp"
#include "mat3.hpp"
#include "mat4.hpp"

struct Triangle;

constexpr float PI = 3.14159265f;
constexpr float EPSILON = 0.0001f;

/*
	Functions
*/

namespace pixl
{
  float sign(float value);

  Vec3f sign(Vec3f value);

  float abs(float value);

  Vec3f abs(Vec3f value);

  float radians(float degrees);

  int16 clamp(int16 value, int16 max);

  float dot(Vec3f vec1, Vec3f vec2);

  Vec3f cross(const Vec3f &a, const Vec3f &b);

  float max(float a, float b);

  double max(double a, double b);

  int32 max(int32 a, int32 b);

  int16 max(int16 a, int16 b);

  uint32 max(uint32 a, uint32 b);

  Vec3f max_component_wise(Vec3f &a, Vec3f &b);

  float min(float a, float b);

  double min(double a, double b);

  uint32 min(uint32 a, uint32 b);

  uint16 min(uint16 a, uint16 b);

  Vec3f min_component_wise(Vec3f &a, Vec3f &b);

  float step(float edge, float x);

  Vec3f step(Vec3f edge, Vec3f x);

  float ceil(float num);

  void swap(float *v1, float *v2);

  Vec3f normalize(Vec3f vec);

  float random_number_normalized_PCG(pcg32_random_t *rngptr);

  Vec2f random_Vec2f_PCG(pcg32_random_t *rngptr);

  Vec3f random_Vec3f_PCG(pcg32_random_t *rngptr);

  Vec3f map_to_unit_sphere(Vec2f vec2);

  Vec3f map_to_unit_hemisphere_cosine_weighted_criver(Vec2f uv, Vec3f normal);

  Vec3f map_to_triangle(Vec2f vec2, Triangle tri);

  Vec3f reflect(Vec3f dir, Vec3f normal);

  void orthonormal_basis(Vec3f &n, Vec3f &t, Vec3f &bt);

  Mat3f construct_TNB_matrix(Vec3f &n);
}

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f &lhs, const Vec3f &rhs);

bool operator!=(const Vec3f &lhs, const Vec3f &rhs);
