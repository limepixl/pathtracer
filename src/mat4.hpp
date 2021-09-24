#pragma once
#include "vec.hpp"

// Represents a square matrix with 
// 4 rows and 4 columns
struct Mat4f
{
	union
	{
		Vec4f rows[4];
		struct
		{
			Vec4f r0, r1, r2, r3;
		};
	};
};

Mat4f CreateMat4f(Vec4f r0, Vec4f r1, Vec4f r2, Vec4f r3);
Mat4f CreateIdentityMat4f();
Mat4f TranslationMat4f(Vec3f translationVec, Mat4f mat);
Mat4f ScaleMat4f(Vec3f scaleVec, Mat4f mat);

Vec4f operator*(Mat4f lhs, Vec4f rhs);
Vec3f operator*(Mat4f lhs, Vec3f rhs);