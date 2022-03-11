#pragma once
#include "../defines.hpp"
#include "mat4.hpp"
#include "mat3.hpp"
#include <cstdlib>
#include "../../thirdparty/pcg-c-basic-0.9/pcg_basic.h"

/*
	Functions
*/

float32 Sign(float32 value);
Vec3f Sign(Vec3f value);

float32 Abs(float32 value);
Vec3f Abs(Vec3f value);

int16 Clamp(int16 value, int16 max);

float32 Dot(Vec3f vec1, Vec3f vec2);

Vec3f Cross(Vec3f a, Vec3f b);

float32 Max(float32 a, float32 b);
int32 Max(int32 a, int32 b);
int16 Max(int16 a, int16 b);
uint32 Max(uint32 a, uint32 b);

Vec3f MaxComponentWise(Vec3f a, Vec3f b);

float32 Min(float32 a, float32 b);
uint32 Min(uint32 a, uint32 b);
uint16 Min(uint16 a, uint16 b);

Vec3f MinComponentWise(Vec3f a, Vec3f b);

float32 Step(float32 edge, float32 x);
Vec3f Step(Vec3f edge, Vec3f x);

float32 Ceil(float32 num);

void Swap(float32 *v1, float32 *v2);

Vec3f NormalizeVec3f(Vec3f vec);

// float32 RandomNumberNormalized();
// Vec2f RandomVec2f();

float32 RandomNumberNormalizedPCG(pcg32_random_t *rngptr);
Vec2f RandomVec2fPCG(pcg32_random_t *rngptr);

Vec3f MapToUnitSphere(Vec2f vec2);
Vec3f MapToUnitHemisphereCosineWeightedCriver(Vec2f uv, Vec3f normal);
Vec3f MapToTriangle(Vec2f vec2, struct Triangle tri);

Vec3f Reflect(Vec3f dir, Vec3f normal);

float32 BalanceHeuristic(float32 pdf_a, float32 pdf_b);

void OrthonormalBasis(Vec3f &n, Vec3f &t, Vec3f &bt);
Mat3f ConstructTNB(Vec3f &n);

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f &lhs, const Vec3f &rhs);
bool operator!=(const Vec3f &lhs, const Vec3f &rhs);