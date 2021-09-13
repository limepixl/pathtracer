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

Vec3f MapToUnitSphere(Vec2f uv)
{
	float32 cos_theta = 2.0f*uv.x-1.0f;
    float32 phi = 2.0f*PI*uv.y;
    float32 sin_theta = sqrtf(1.0f-cos_theta*cos_theta);
    float32 sin_phi = sinf(phi);
    float32 cos_phi = cosf(phi);
    
    return { sin_theta*cos_phi, cos_theta, sin_theta*sin_phi };
}

Vec3f MapToUnitHemisphereCosineWeighted(Vec2f uv, Vec3f normal)
{
    Vec3f p = MapToUnitSphere(uv);
    return normal+p;
}