#include "mat4.hpp"

Mat4f TranslationMat4f(Vec3f translation_vec, Mat4f mat)
{
  mat.r0.w += translation_vec.x;
  mat.r1.w += translation_vec.y;
  mat.r2.w += translation_vec.z;
  return mat;
}

Mat4f ScaleMat4f(Vec3f scale_vec, Mat4f mat)
{
  mat.r0.x *= scale_vec.x;
  mat.r1.y *= scale_vec.y;
  mat.r2.z *= scale_vec.z;
  return mat;
}

Vec4f operator*(Mat4f lhs, const Vec4f &rhs)
{
  return {lhs.r0.x * rhs.x + lhs.r0.y * rhs.y + lhs.r0.z * rhs.z + lhs.r0.w * rhs.w,
          lhs.r1.x * rhs.x + lhs.r1.y * rhs.y + lhs.r1.z * rhs.z + lhs.r1.w * rhs.w,
          lhs.r2.x * rhs.x + lhs.r2.y * rhs.y + lhs.r2.z * rhs.z + lhs.r2.w * rhs.w,
          lhs.r3.x * rhs.x + lhs.r3.y * rhs.y + lhs.r3.z * rhs.z + lhs.r3.w * rhs.w};
}

Vec3f operator*(Mat4f lhs, const Vec3f &rhs)
{
  Vec4f tmp_vec(rhs.x, rhs.y, rhs.z, 1.0f);
  tmp_vec = lhs * tmp_vec;
  return {tmp_vec.x, tmp_vec.y, tmp_vec.z};
}

Mat4f::Mat4f()
{
  r0 = Vec4f(1.0f, 0.0f, 0.0f, 0.0f);
  r1 = Vec4f(0.0f, 1.0f, 0.0f, 0.0f);
  r2 = Vec4f(0.0f, 0.0f, 1.0f, 0.0f);
  r3 = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4f::Mat4f(Vec4f r0, Vec4f r1, Vec4f r2, Vec4f r3)
    : r0(r0), r1(r1), r2(r2), r3(r3) {}
