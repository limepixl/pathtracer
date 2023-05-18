#pragma once
#include "../defines.hpp"

struct Vec4f
{
    float x, y, z, w;

    Vec4f() = default;

    explicit Vec4f(float v);

    Vec4f(float x, float y, float z, float w);
};

Vec4f operator/(const Vec4f &lhs, float rhs);

struct Vec3f
{
    float x, y, z;

    Vec3f();

    explicit Vec3f(float v);

    Vec3f(float x, float y, float z);
};

Vec3f operator+(const Vec3f &lhs, const Vec3f &rhs);

Vec3f operator-(const Vec3f &lhs, const Vec3f &rhs);

Vec3f operator-(const Vec3f &vec);

Vec3f operator*(float lhs, const Vec3f &rhs);

Vec3f operator*(const Vec3f &lhs, float rhs);

Vec3f operator*(const Vec3f &lhs, const Vec3f &rhs);

Vec3f operator/(const Vec3f &lhs, float rhs);

Vec3f operator/(float lhs, const Vec3f &rhs);

Vec3f operator+=(Vec3f &lhs, const Vec3f &rhs);

Vec3f operator*=(Vec3f &lhs, float rhs);

Vec3f operator/=(Vec3f &lhs, float rhs);

Vec3f operator*=(Vec3f &lhs, const Vec3f &rhs);

bool operator<=(const Vec3f &lhs, const Vec3f &rhs);

bool operator>=(const Vec3f &lhs, const Vec3f &rhs);

bool operator<(const Vec3f &lhs, const Vec3f &rhs);

bool operator>(const Vec3f &lhs, const Vec3f &rhs);

struct Vec2f
{
    float x, y;

    Vec2f();

    explicit Vec2f(float v);

    Vec2f(float x, float y);
};

Vec2f operator-(const Vec2f &lhs, const Vec2f &rhs);

Vec2f operator+(const Vec2f &lhs, const Vec2f &rhs);

Vec2f operator*(float lhs, const Vec2f &rhs);
