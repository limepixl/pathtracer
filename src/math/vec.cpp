#include "vec.hpp"

Vec4f operator/(const Vec4f &lhs, float rhs)
{
	return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs};
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

bool operator<(const Vec3f &lhs, const Vec3f &rhs)
{
    return (lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z);
}

bool operator>(const Vec3f &lhs, const Vec3f &rhs)
{
    return (lhs.x > rhs.x && lhs.y > rhs.y && lhs.z > rhs.z);
}

Vec2f operator-(const Vec2f &lhs, const Vec2f &rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

//Vec2f operator+(const Vec2f lhs, const Vec2f rhs)
//{
//	return { lhs.x + rhs.x, lhs.y + rhs.y };
//}

//Vec2f operator*(const float lhs, const Vec2f rhs)
//{
//	return { lhs * rhs.x, lhs * rhs.y };
//}

Vec4f::Vec4f(float v)
        : x(v), y(v), z(v), w(v) {}

Vec4f::Vec4f(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w) {}

Vec3f::Vec3f(float v)
        : x(v), y(v), z(v) {}

Vec3f::Vec3f(float x, float y, float z)
        : x(x), y(y), z(z) {}

Vec3f::Vec3f()
        : x(0.0f), y(0.0f), z(0.0f) {}

Vec2f::Vec2f(float v)
        : x(v), y(v) {}

Vec2f::Vec2f(float x, float y)
        : x(x), y(y) {}

Vec2f::Vec2f()
        : x(0.0f), y(0.0f) {}
