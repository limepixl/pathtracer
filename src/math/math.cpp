#include "math.hpp"
#include "../scene/triangle.hpp"
#include <cmath>

/*
	Functions
*/

float pixl::sign(float value)
{
  if (value < 0.0f)
  {
    return -1.0f;
  }

  if (value > 0.0f)
  {
    return 1.0f;
  }

  return value;
}

Vec3f pixl::sign(Vec3f value)
{
  return {pixl::sign(value.x), pixl::sign(value.y), pixl::sign(value.z)};
}

float pixl::abs(float value)
{
  if (value < 0.0f)
  {
    return -value;
  }

  return value;
}

Vec3f pixl::abs(Vec3f value)
{
  return {abs(value.x), abs(value.y), abs(value.z)};
}

float pixl::radians(float degrees)
{
  return degrees * PI / 180.0f;
}

int16 pixl::clamp(int16 value, int16 max)
{
  if (value > max)
  {
    return max;
  }

  return value;
}

float pixl::dot(Vec3f vec1, Vec3f vec2)
{
  return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

Vec3f pixl::cross(const Vec3f &a, const Vec3f &b)
{
  float x = a.y * b.z - a.z * b.y;
  float y = a.z * b.x - a.x * b.z;
  float z = a.x * b.y - a.y * b.x;
  return {x, y, z};
}

float pixl::max(float a, float b)
{
  return a > b ? a : b;
}

double pixl::max(double a, double b)
{
  return a > b ? a : b;
}

int32 pixl::max(int32 a, int32 b)
{
  return a > b ? a : b;
}

int16 pixl::max(int16 a, int16 b)
{
  return a > b ? a : b;
}

uint32 pixl::max(uint32 a, uint32 b)
{
  return a > b ? a : b;
}

Vec3f pixl::max_component_wise(Vec3f &a, Vec3f &b)
{
  return {pixl::max(a.x, b.x), pixl::max(a.y, b.y), pixl::max(a.z, b.z)};
}

float pixl::min(float a, float b)
{
  return a < b ? a : b;
}

double pixl::min(double a, double b)
{
  return a < b ? a : b;
}

uint32 pixl::min(uint32 a, uint32 b)
{
  return a < b ? a : b;
}

uint16 pixl::min(uint16 a, uint16 b)
{
  return a < b ? a : b;
}

Vec3f pixl::min_component_wise(Vec3f &a, Vec3f &b)
{
  return {pixl::min(a.x, b.x), pixl::min(a.y, b.y), pixl::min(a.z, b.z)};
}

float pixl::step(float edge, float x)
{
  return x < edge ? 0.0f : 1.0f;
}

Vec3f pixl::step(Vec3f edge, Vec3f x)
{
  return {pixl::step(edge.x, x.x), pixl::step(edge.y, x.y), pixl::step(edge.z, x.z)};
}

float pixl::ceil(float num)
{
  if (num - (float) ((int32) num) > 0.0f)
  {
    return float((int32) num + 1);
  }

  return num;
}

void pixl::swap(float *v1, float *v2)
{
  float tmp = *v1;
  *v1 = *v2;
  *v2 = tmp;
}

Vec3f pixl::normalize(Vec3f vec)
{
  return vec / sqrtf(pixl::dot(vec, vec));
}

// PCG variants of the above functions
float pixl::random_number_normalized_PCG(pcg32_random_t *rngptr)
{
  double d = ldexp(pcg32_random_r(rngptr), -32);
  return (float) d;
}

Vec2f pixl::random_Vec2f_PCG(pcg32_random_t *rngptr)
{
  float r1 = random_number_normalized_PCG(rngptr);
  while (r1 < 0.0001f || r1 > 0.9999f)
  {
    r1 = random_number_normalized_PCG(rngptr);
  }

  float r2 = random_number_normalized_PCG(rngptr);
  while (r2 < 0.0001f || r2 > 0.9999f)
  {
    r2 = random_number_normalized_PCG(rngptr);
  }

  return {r1, r2};
}

Vec3f pixl::random_Vec3f_PCG(pcg32_random_t *rngptr)
{
  float r1 = random_number_normalized_PCG(rngptr);
  while (r1 < 0.0001f || r1 > 0.9999f)
  {
    r1 = random_number_normalized_PCG(rngptr);
  }

  float r2 = random_number_normalized_PCG(rngptr);
  while (r2 < 0.0001f || r2 > 0.9999f)
  {
    r2 = random_number_normalized_PCG(rngptr);
  }

  float r3 = random_number_normalized_PCG(rngptr);
  while (r1 < 0.0001f || r1 > 0.9999f)
  {
    r1 = random_number_normalized_PCG(rngptr);
  }

  return {r1, r2, r3};
}

Vec3f pixl::map_to_unit_sphere(Vec2f vec2)
{
  // First we map [0,1] to [0,2] and subtract one to map
  // that to [-1, 1], which is the range of cosine.
  float cosTheta = 2.0f * vec2.x - 1.0f;

  // We can directly map phi to [0, 2PI] from [0, 1] by just
  // multiplying it with 2PI
  float Phi = 2.0f * PI * vec2.y;

  // sin^2(x) = 1 - cos^2(x)
  // sin(x) = sqrt(1 - cos^2(x))
  float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

  float sinPhi = sinf(Phi);
  float cosPhi = cosf(Phi);

  // Just a conversion between spherical and Cartesian coordinates
  return {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
}

Vec3f pixl::map_to_unit_hemisphere_cosine_weighted_criver(Vec2f uv, Vec3f normal)
{
  Vec3f p = map_to_unit_sphere(uv);
  return normalize(p + normal);
}

Vec3f pixl::map_to_triangle(Vec2f vec2, Triangle tri)
{
  float u = vec2.x;
  float v = vec2.y;

  if (u + v > 1.0f)
  {
    // The generated point is outside triangle but
    // within the parallelogram defined by the 2 edges
    // of the triangle (v1-v0 and v2-v0)
    u = 1.0f - u;
    v = 1.0f - v;
  }

  Vec3f p = u * tri.edge1 + v * tri.edge2;
  return p + tri.v0;
}

Vec3f pixl::reflect(Vec3f dir, Vec3f normal)
{
  return 2.0f * dot(normal, dir) * normal - dir;
}

// https://jcgt.org/published/0006/01/01/
void pixl::orthonormal_basis(Vec3f &n, Vec3f &t, Vec3f &bt)
{
  float sign = copysignf(1.0f, n.z);
  float a = -1.0f / (sign + n.z);
  float b = n.x * n.y * a;

  t = Vec3f(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
  bt = Vec3f(b, sign + n.y * n.y * a, -n.y);
}

Mat3f pixl::construct_TNB_matrix(Vec3f &n)
{
  Vec3f t {}, bt {};
  orthonormal_basis(n, t, bt);
  Mat3f res(t, n, bt);
  return TransposeMat3f(res);
}

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f &lhs, const Vec3f &rhs)
{
  Vec3f grace_interval(EPSILON);
  Vec3f absdiff = pixl::abs(lhs - rhs);
  return absdiff <= grace_interval;
}

bool operator!=(const Vec3f &lhs, const Vec3f &rhs)
{
  return !(lhs == rhs);
}
