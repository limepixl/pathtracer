#pragma once
#include "vec.hpp"

struct Mat3f
{
	union
	{
		Vec3f rows[3];
		struct
		{
			Vec3f r0, r1, r2;
		};
	};

	Mat3f();
	Mat3f(Vec3f r0, Vec3f r1, Vec3f r2);
};

Mat3f TransposeMat3f(Mat3f mat);

Vec3f operator*(Mat3f lhs, const Vec3f &rhs);
