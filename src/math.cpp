#define _CRT_RAND_S
#include "math.hpp"
#include <math.h>
#include "intersect.hpp"

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

int16 Max(int16 a, int16 b)
{
	return a > b ? a : b;
}

uint32 Max(uint32 a, uint32 b)
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

uint32 Min(uint32 a, uint32 b)
{
	return a < b ? a : b;
}

uint16 Min(uint16 a, uint16 b)
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

float32 Ceil(float32 num)
{
	if(num - (int32)num > 0.0f)
		return float32((int32)num + 1);
	
	return num;
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

#include "../pcg-c-basic-0.9/pcg_basic.h"

// Returns a number in (0, 1)
float32 RandomNumberNormalized()
{
	unsigned int val = 0;
	rand_s(&val);

	float64 result = (float64)(val+1000.0) / ((float64)UINT_MAX + 2000.0);
	return (float32)result;
}

Vec2f RandomVec2f()
{
	return { RandomNumberNormalized(), RandomNumberNormalized() };
}

// PCG variants of the above functions
float32 RandomNumberNormalizedPCG(pcg32_random_t *rngptr)
{
	/*
	uint32 val = pcg32_random_r(rngptr);

	float64 result = (float64)(val+1000.0) / ((float64)UINT_MAX + 2000.0);
	*/
	
	float64 d = ldexp(pcg32_random_r(rngptr), -32);
	return (float32)d;
}

Vec2f RandomVec2fPCG(pcg32_random_t *rngptr)
{
	return { RandomNumberNormalizedPCG(rngptr), RandomNumberNormalizedPCG(rngptr) };
}

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
	return NormalizeVec3f(p+normal);
}

Vec3f MapToTriangle(Vec2f vec2, Triangle tri)
{
	float32 u = vec2.x;
	float32 v = vec2.y;
	
	if(u + v > 1.0f)
	{
		// The generated point is outside triangle but
		// within the parallelogram defined by the 2 edges
		// of the triangle (v1-v0 and v2-v0)
		u = 1.0f - u;
		v = 1.0f - v;
	}
	
	Vec3f p = u * tri.edge1 + v * tri.edge2;
	return p + tri.v0;
}

Vec3f Reflect(Vec3f dir, Vec3f normal)
{
	return 2.0f * Dot(normal, dir) * normal - dir;
}

float32 BalanceHeuristic(float32 pdf_a, float32 pdf_b)
{
	return pdf_a / (pdf_a + pdf_b);
}

// https://jcgt.org/published/0006/01/01/
void OrthonormalBasis(Vec3f &n, Vec3f &t, Vec3f &bt)
{
	float32 sign = copysignf(1.0f, n.z);
	float32 a = -1.0f / (sign + n.z);
	float32 b = n.x * n.y * a;

	t = CreateVec3f(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
	bt = CreateVec3f(b, sign + n.y * n.y * a, -n.y);
}

Mat3f ConstructTNB(Vec3f &n)
{
	Vec3f t = {}, bt = {};
	OrthonormalBasis(n, t, bt);
	Mat3f res = CreateMat3f(t, n, bt);
	return TransposeMat3f(res);
}

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f &lhs, const Vec3f &rhs)
{
	Vec3f graceInterval = {FLOAT_EQUALITY_PRECISION, 
						   FLOAT_EQUALITY_PRECISION, 
						   FLOAT_EQUALITY_PRECISION};
	Vec3f absdiff = Abs(lhs - rhs);
	return absdiff <= graceInterval;
}

bool operator!=(const Vec3f &lhs, const Vec3f &rhs)
{
	return !(lhs == rhs);
}