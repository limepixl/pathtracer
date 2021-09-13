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

Vec3f operator+=(Vec3f &lhs, const Vec3f rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
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

inline float32 Dot(Vec3f vec1, Vec3f vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

inline Vec3f NormalizeVec3f(Vec3f vec)
{
	return vec / sqrtf(Dot(vec, vec));
}

float32 RandomNumberNormalized()
{
	return (float32)((float64)rand() / (float64)RAND_MAX);
}

// TODO: create my own random function
Vec2f RandomVec2f()
{
	return { RandomNumberNormalized(), RandomNumberNormalized() };
}

// Maps 2 uniformly sampled random numbers from [0, 1]
// to corresponding theta and pfi values for a direction
// in spherical coordinates. This can be converted to a 
// Vec3f for a normal direction with 3 components.
Vec3f MapToUnitHemisphereUniformly(Vec2f vec2)
{
	// Phi has a range [0, 2PI]
	//float32 phi = 2.0f * PI * vec2.y;
	//float32 z = vec2.x;
	//float32 r = sqrtf();
	return {};
}

// TODO: deeper understanding of this
Vec3f MapToUnitHemisphereCosineWeighted(Vec2f vec2)
{
	float32 m = 5.0f;
	float32 theta = acosf(powf(1.0f - vec2.x, 1.0f / (1.0f + m)));
    float32 phi = 2.0f * PI * vec2.y;

	// Convert hemispherical coordinates to Cartesian coordinates
    float32 x = sinf(theta) * cosf(phi);
    float32 y = sinf(theta) * sinf(phi);
	float32 z = cosf(theta);
	return {x, y, z};
}