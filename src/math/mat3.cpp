#include "mat3.hpp"

Mat3f TransposeMat3f(Mat3f mat)
{
	return Mat3f(
		Vec3f(mat.r0.x, mat.r1.x, mat.r2.x),
		Vec3f(mat.r0.y, mat.r1.y, mat.r2.y),
		Vec3f(mat.r0.z, mat.r1.z, mat.r2.z));
}

Vec3f operator*(Mat3f lhs, const Vec3f &rhs)
{
	return Vec3f(lhs.r0.x * rhs.x + lhs.r0.y * rhs.y + lhs.r0.z * rhs.z,
				 lhs.r1.x * rhs.x + lhs.r1.y * rhs.y + lhs.r1.z * rhs.z,
				 lhs.r2.x * rhs.x + lhs.r2.y * rhs.y + lhs.r2.z * rhs.z);
}

Mat3f::Mat3f()
{
	r0 = Vec3f(1.0f, 0.0f, 0.0f);
	r1 = Vec3f(0.0f, 1.0f, 0.0f);
	r2 = Vec3f(0.0f, 0.0f, 1.0f);
}

Mat3f::Mat3f(Vec3f r0, Vec3f r1, Vec3f r2)
	: r0(r0), r1(r1), r2(r2)
{}
