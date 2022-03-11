#pragma once
#include "../defines.hpp"

struct Vec4f
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		struct
		{
			float r, g, b, a;
		};
		float values[4];
	};
};

Vec4f CreateVec4f(float x, float y, float z, float w);
Vec4f CreateVec4f(float v);

struct Vec3f
{
	union
	{
		float values[3];
		struct
		{
			float x, y, z;
		};
	};
};

Vec3f CreateVec3f(float x, float y, float z);
Vec3f CreateVec3f(float v);
Vec3f operator+(const Vec3f &lhs, const Vec3f &rhs);
Vec3f operator-(const Vec3f &lhs, const Vec3f &rhs);
Vec3f operator-(const Vec3f &vec);
Vec3f operator*(const float lhs, const Vec3f &rhs);
Vec3f operator*(const Vec3f &lhs, const float rhs);
Vec3f operator*(const Vec3f &lhs, const Vec3f &rhs);
Vec3f operator/(const Vec3f &lhs, const float rhs);
Vec3f operator/(const float lhs, const Vec3f &rhs);
Vec3f operator+=(Vec3f &lhs, const Vec3f &rhs);
Vec3f operator*=(Vec3f &lhs, const float rhs);
Vec3f operator/=(Vec3f &lhs, const float rhs);
Vec3f operator*=(Vec3f &lhs, const Vec3f &rhs);
bool operator<=(const Vec3f &lhs, const Vec3f &rhs);
bool operator>=(const Vec3f &lhs, const Vec3f &rhs);

struct Vec2f
{
	union
	{
		float values[2];
		struct
		{
			float x, y;
		};
	};
};

Vec2f CreateVec2f(float x, float y);
Vec2f CreateVec2f(float v);
Vec2f operator-(const Vec2f &lhs, const Vec2f &rhs);
Vec2f operator+(const Vec2f &lhs, const Vec2f &rhs);
Vec2f operator*(const float lhs, const Vec2f &rhs);