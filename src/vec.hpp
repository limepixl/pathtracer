#pragma once
#include "defines.hpp"

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

Vec3f CreateVec3f(float32 x, float32 y, float32 z);
Vec3f CreateVec3f(float32 v);
Vec3f operator+(const Vec3f lhs, const Vec3f rhs);
Vec3f operator-(const Vec3f lhs, const Vec3f rhs);
Vec3f operator-(const Vec3f vec);
Vec3f operator*(const float32 lhs, const Vec3f rhs);
Vec3f operator*(const Vec3f lhs, const float32 rhs);
Vec3f operator*(const Vec3f lhs, const Vec3f rhs);
Vec3f operator/(const Vec3f lhs, const float32 rhs);
Vec3f operator/(const float32 lhs, const Vec3f rhs);
Vec3f operator+=(Vec3f &lhs, const Vec3f rhs);
Vec3f operator*=(Vec3f &lhs, const float32 rhs);
Vec3f operator/=(Vec3f &lhs, const float32 rhs);
Vec3f operator*=(Vec3f &lhs, const Vec3f rhs);
bool operator<=(Vec3f lhs, Vec3f rhs);

struct Vec2f
{
	union
	{
		float32 values[2];
		struct
		{
			float32 x, y;
		};
	};
};

Vec2f CreateVec2f(float32 x, float32 y);
Vec2f CreateVec2f(float32 v);
Vec2f operator-(const Vec2f lhs, const Vec2f rhs);
Vec2f operator+(const Vec2f lhs, const Vec2f rhs);
Vec2f operator*(const float32 lhs, const Vec2f rhs);