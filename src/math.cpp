#define _CRT_RAND_S
#include "math.hpp"
#include <math.h>

/*
	Functions
*/

float32 Sign(float32 value)
{
	if(value < 0.0f)
		return -1.0f;
	
	if(value > 0.0f)
		return 1.0f;
	
	return value;
}

Vec3f Sign(Vec3f value)
{
	return {Sign(value.x), Sign(value.y), Sign(value.z)};
}

float32 Abs(float32 value)
{
	if(value < 0.0f)
		return -value;
	
	return value;
}

Vec3f Abs(Vec3f value)
{
	return {Abs(value.x), Abs(value.y), Abs(value.z)};
}

int16 Clamp(int16 value, int16 max)
{
	if(value > max)
		return max;

	return value;
}

float32 Dot(Vec3f vec1, Vec3f vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

Vec3f Cross(Vec3f a, Vec3f b)
{
	float32 x = a.y*b.z - a.z*b.y;
	float32 y = a.z*b.x - a.x*b.z;
	float32 z = a.x*b.y - a.y*b.x;
	return {x, y, z};
}

float32 Max(float32 a, float32 b)
{
	return a > b ? a : b;
}

int32 Max(int32 a, int32 b)
{
	return a > b ? a : b;
}

Vec3f MaxComponentWise(Vec3f a, Vec3f b)
{
	return { Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z) };
}

float32 Min(float32 a, float32 b)
{
	return a < b ? a : b;
}

Vec3f MinComponentWise(Vec3f a, Vec3f b)
{
	return { Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z) };
}

float32 Step(float32 edge, float32 x)
{
	return x < edge ? 0.0f : 1.0f;
}

Vec3f Step(Vec3f edge, Vec3f x)
{
	return {Step(edge.x, x.x), Step(edge.y, x.y), Step(edge.z, x.z)};
}

void Swap(float32 *v1, float32 *v2)
{
	float32 tmp = *v1;
	*v1 = *v2;
	*v2 = tmp;
}

Vec3f NormalizeVec3f(Vec3f vec)
{
	return vec / sqrtf(Dot(vec, vec));
}

#include <stdio.h>

// Returns a number in (0, 1)
float32 RandomNumberNormalized()
{
	unsigned int val = 0;
	rand_s(&val);

	float64 result = (float64)(val+1000.0) / ((float64)UINT_MAX + 2000.0);
	return (float32)result;
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

Vec3f MapToUnitHemisphereCosineWeightedCriver(Vec2f uv, Vec3f normal)
{
    Vec3f p = MapToUnitSphere(uv);
	return p+normal;
}

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f lhs, const Vec3f rhs)
{
	Vec3f graceInterval = {FLOAT_EQUALITY_PRECISION, 
						   FLOAT_EQUALITY_PRECISION, 
						   FLOAT_EQUALITY_PRECISION};
	Vec3f absdiff = Abs(lhs - rhs);
	return absdiff <= graceInterval;
}

bool operator!=(const Vec3f lhs, const Vec3f rhs)
{
	return !(lhs == rhs);
}