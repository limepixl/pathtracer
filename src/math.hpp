#pragma once
#include "defines.hpp"
#include "mat4.hpp"
#include <cstdlib>

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

Vec3f MaxComponentWise(Vec3f a, Vec3f b);

float32 Min(float32 a, float32 b);

Vec3f MinComponentWise(Vec3f a, Vec3f b);

float32 Step(float32 edge, float32 x);
Vec3f Step(Vec3f edge, Vec3f x);

void Swap(float32 *v1, float32 *v2);

Vec3f NormalizeVec3f(Vec3f vec);

// TODO: create my own random function
float32 RandomNumberNormalized();
Vec2f RandomVec2f();

Vec3f MapToUnitSphere(Vec2f vec2);
Vec3f MapToUnitHemisphereCosineWeightedCriver(Vec2f uv, Vec3f normal);

/*
	Operators that use the utility functions above
*/

bool operator==(const Vec3f lhs, const Vec3f rhs);
bool operator!=(const Vec3f lhs, const Vec3f rhs);