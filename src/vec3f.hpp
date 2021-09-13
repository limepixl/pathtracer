#pragma once
#include <math.h>

struct Vec3f
{
	union
	{
		float32 values[3];
		struct
		{
			float32 x, y, z;
		};
	};
};

Vec3f operator+(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3f operator-(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3f operator*(const float32 lhs, const Vec3f rhs)
{
	return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs};
}

Vec3f operator*(const Vec3f lhs, const float32 rhs)
{
	return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Vec3f operator/(const Vec3f lhs, const float32 rhs)
{
	return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

Vec3f operator+=(Vec3f &lhs, const Vec3f rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

/*
	Functions
*/

inline float32 Dot(Vec3f vec1, Vec3f vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

inline Vec3f NormalizeVec3f(Vec3f vec)
{
	return vec / sqrtf(Dot(vec, vec));
}