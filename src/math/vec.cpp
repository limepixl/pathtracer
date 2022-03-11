#include "vec.hpp"
#include "../defines.hpp"

Vec4f CreateVec4f(float x, float y, float z, float w)
{
	Vec4f result = {x, y, z, w};
	return result;
}

Vec4f CreateVec4f(float v)
{
	Vec4f result = {v, v, v, v};
	return result;
}

Vec3f CreateVec3f(float x, float y, float z)
{
	Vec3f result = {x, y, z};
	return result;
}

Vec3f CreateVec3f(float v)
{
	Vec3f result = {v, v, v};
	return result;
}

Vec3f operator+(const Vec3f &lhs, const Vec3f &rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3f operator-(const Vec3f &lhs, const Vec3f &rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3f operator-(const Vec3f &vec)
{
	return {-vec.x, -vec.y, -vec.z};
}

Vec3f operator*(const float lhs, const Vec3f &rhs)
{
	return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs};
}

Vec3f operator*(const Vec3f &lhs, const float rhs)
{
	return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Vec3f operator*(const Vec3f &lhs, const Vec3f &rhs)
{
	return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

Vec3f operator/(const Vec3f &lhs, const float rhs)
{
	return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

Vec3f operator/(const float lhs, const Vec3f &rhs)
{
	return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};
}

Vec3f operator+=(Vec3f &lhs, const Vec3f &rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

Vec3f operator*=(Vec3f &lhs, const float rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	return lhs;
}

Vec3f operator/=(Vec3f &lhs, const float rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;
	return lhs;
}

Vec3f operator*=(Vec3f &lhs, const Vec3f &rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

bool operator<=(const Vec3f &lhs, const Vec3f &rhs)
{
	return (lhs.x <= rhs.x && lhs.y <= rhs.y && lhs.z <= rhs.z);
}

bool operator>=(const Vec3f &lhs, const Vec3f &rhs)
{
	return (lhs.x >= rhs.x && lhs.y >= rhs.y && lhs.z >= rhs.z);
}

Vec2f CreateVec2f(float x, float y)
{
	return {x, y};
}

Vec2f CreateVec2f(float v)
{
	return {v, v};
}

Vec2f operator-(const Vec2f &lhs, const Vec2f &rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Vec2f operator+(const Vec2f lhs, const Vec2f rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Vec2f operator*(const float lhs, const Vec2f rhs)
{
	return {lhs * rhs.x, lhs * rhs.y};
}