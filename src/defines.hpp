#pragma once

// NOTE: This only holds for LLP64
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef char  int8;
typedef short int16;
typedef int   int32;
typedef long  int64;
// NOTE: This only holds for LLP64

constexpr uint32 WIDTH = 1280;
constexpr uint32 HEIGHT = 720;

constexpr uint8 BVH_NUM_LEAF_TRIS = 8;

// ASSERT writes to 0x0, which will throw an access
// violation exception. ASSERT is used when we want
// some expression to be different from zero.
#define ASSERT(expression) \
	if (!(expression))     \
	{                      \
		*(int *)NULL = 0;  \
	}
