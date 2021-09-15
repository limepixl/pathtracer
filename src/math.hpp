#pragma once
#include <math.h>
#include <stdlib.h>

/*
	Vec structs and their operators
*/

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

Vec3f CreateVec3f(float32 x, float32 y, float32 z)
{
	return {x, y, z};
}

Vec3f operator+(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3f operator-(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3f operator-(const Vec3f vec)
{
	return {-vec.x, -vec.y, -vec.z};
}

Vec3f operator*(const float32 lhs, const Vec3f rhs)
{
	return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs};
}

Vec3f operator*(const Vec3f lhs, const float32 rhs)
{
	return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

Vec3f operator*(const Vec3f lhs, const Vec3f rhs)
{
	return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

Vec3f operator/(const Vec3f lhs, const float32 rhs)
{
	return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

Vec3f operator/(const float32 lhs, const Vec3f rhs)
{
	return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};
}

Vec3f operator+=(Vec3f &lhs, const Vec3f rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

Vec3f operator*=(Vec3f &lhs, const float32 rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	return lhs;
}

Vec3f operator/=(Vec3f &lhs, const float32 rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;
	return lhs;
}

Vec3f operator*=(Vec3f &lhs, const Vec3f rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

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

Vec2f operator-(const Vec2f lhs, const Vec2f rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Vec2f operator+(const Vec2f lhs, const Vec2f rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

/*
	Functions
*/

inline float32 Sign(float32 value)
{
	if(value < 0.0f)
		return -1.0f;
	
	if(value > 0.0f)
		return 1.0f;
	
	return value;
}

inline Vec3f Sign(Vec3f value)
{
	return {Sign(value.x), Sign(value.y), Sign(value.z)};
}

inline int16 Clamp(int16 value, int16 max)
{
	if(value > max)
		return max;

	return value;
}

inline float32 Dot(Vec3f vec1, Vec3f vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

inline float32 Max(float32 a, float32 b)
{
	return a > b ? a : b;
}

inline float32 Min(float32 a, float32 b)
{
	return a < b ? a : b;
}

inline float32 Step(float32 edge, float32 x)
{
	return x < edge ? 0.0f : 1.0f;
}

inline Vec3f Step(Vec3f edge, Vec3f x)
{
	return {Step(edge.x, x.x), Step(edge.y, x.y), Step(edge.z, x.z)};
}

inline Vec3f NormalizeVec3f(Vec3f vec)
{
	return vec / sqrtf(Dot(vec, vec));
}

float32 RandomNumberNormalized()
{
	return (float32)((float64)rand() / (float64)(RAND_MAX + 1));
}

// TODO: create my own random function
Vec2f RandomVec2f()
{
	return { RandomNumberNormalized(), RandomNumberNormalized() };
}

// TODO: understand the below functions
Vec3f MapToUnitSphere(Vec2f vec2)
{
	// First we map [0,1] to [0,2] and subtract one to map
	// that to [-1, 1], which is the range of cosine.
	float32 cosTheta = 2.0f * vec2.x - 1.0f;

	// We can directly map phi to [0, 2PI] from [0, 1] by just 
	// multiplying it with 2PI
    float32 Phi = 2.0f*PI*vec2.y;

	// sin^2(x) = 1 - cos^2(x)
	// sin(x) = sqrt(1 - cos^2(x))
    float32 sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

    float32 sinPhi = sinf(Phi);
    float32 cosPhi = cosf(Phi);
    
	// Just a conversion between spherical and Cartesian coordinates
    return { sinTheta * cosPhi, cosTheta, sinTheta * sinPhi };
}

Vec3f MapToUnitHemisphereCosineWeighted(Vec2f uv, Vec3f normal)
{
    Vec3f p = MapToUnitSphere(uv);
    return NormalizeVec3f(normal+p);
}