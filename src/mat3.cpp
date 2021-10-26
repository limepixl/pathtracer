#include "mat3.hpp"

Mat3f CreateMat3f(Vec3f r0, Vec3f r1, Vec3f r2)
{
	return {r0, r1, r2};
}

Mat3f CreateIdentityMat3f()
{
	Mat3f res =
	{
		CreateVec3f(1.0f, 0.0f, 0.0f),
		CreateVec3f(0.0f, 1.0f, 0.0f),
		CreateVec3f(0.0f, 0.0f, 1.0f),
	};
	return res;
}

Mat3f TransposeMat3f(Mat3f mat)
{
	return CreateMat3f
	(
		CreateVec3f(mat.r0.x, mat.r1.x, mat.r2.x),
		CreateVec3f(mat.r0.y, mat.r1.y, mat.r2.y),
		CreateVec3f(mat.r0.z, mat.r1.z, mat.r2.z)
	);
}

Vec3f operator*(Mat3f lhs, const Vec3f &rhs)
{
	return CreateVec3f(lhs.r0.x*rhs.x + lhs.r0.y*rhs.y + lhs.r0.z*rhs.z,
					   lhs.r1.x*rhs.x + lhs.r1.y*rhs.y + lhs.r1.z*rhs.z,
					   lhs.r2.x*rhs.x + lhs.r2.y*rhs.y + lhs.r2.z*rhs.z);
}