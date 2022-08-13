#pragma once

// NOTE: This only holds for LLP64
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;
// NOTE: This only holds for LLP64

constexpr uint8 NUM_THREADS = 11;

constexpr uint32 WIDTH = 1280;
constexpr uint32 HEIGHT = 720;

constexpr uint8 BOUNCE_COUNT = 5;
constexpr uint16 NUM_SAMPLES = 1;
constexpr uint8 NUM_SHADOW_RAYS = 1;

constexpr uint8 BVH_NUM_LEAF_TRIS = 8;

constexpr float TMIN = 0.001f;
constexpr float TMAX = 10000.0f;
constexpr float PI = 3.14159265f;
constexpr float EPSILON = 0.000001f;
constexpr float ENVIRONMENT_MAP_LE = 1.0f;

constexpr bool TWO_SIDED_LIGHT = false;

constexpr float IDEAL_SPECULAR_TINT = 1.0f;

constexpr bool PREDICTABLE_RAND = false;

// Macro to return stack allocated array length
#define ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// a == b is transformed into Abs(a-b) <= FLOAT_EQUALITY_PRECISION
constexpr float FLOAT_EQUALITY_PRECISION = EPSILON;

// ASSERT writes to 0x0, which will throw an access
// violation exception. ASSERT is used when we want
// some expression to be different from zero.
#define ASSERT(expression) \
	if (!(expression))     \
	{                      \
		*(int *)NULL = 0;  \
	}
