#pragma once

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

Vec3f operator+(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3f operator-(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3f operator*(const float lhs, const Vec3f rhs)
{
	return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs};
}

Vec3f operator*(const Vec3f lhs, const float rhs)
{
	return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Vec3f operator/(const Vec3f lhs, const float rhs)
{
	return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

Vec3f NormalizeVec3f(Vec3f vec)
{
	return vec / (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}