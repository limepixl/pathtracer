#pragma once

#define NUM_THREADS 11

#define BOUNCE_MIN 0
#define BOUNCE_COUNT 6
#define NUM_BOUNCES BOUNCE_MIN + BOUNCE_COUNT
#define NUM_SAMPLES 64
#define NUM_SHADOW_RAYS 1

#define TMIN 0.001f
#define TMAX 10000.0f
#define PI 3.14159265f
#define EPSILON 0.0001f
#define ENVIRONMENT_MAP_LE 0.0f

#define TWO_SIDED_LIGHT 0

// NOTE: This only holds for LLP64
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

typedef float float32;
typedef double float64;

// Macro to return stack allocated array length
#define ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// a == b is transformed into Abs(a-b) <= FLOAT_EQUALITY_PRECISION
#define FLOAT_EQUALITY_PRECISION 0.0001f

// ASSERT writes to 0x0, which will throw an access 
// violation exception. ASSERT is used when we want
// some expression to be different from zero.
#define ASSERT(expression) \
	if(!(expression)) { *(int *)NULL = 0; }