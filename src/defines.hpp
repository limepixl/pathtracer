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

constexpr uint32 WIDTH = 1280;
constexpr uint32 HEIGHT = 720;
constexpr uint16 FRAMERATE = 120;
constexpr uint32 BOUNCE_COUNT = 5;

constexpr uint32 NUM_WORK_GROUPS_X = WIDTH / 8;
constexpr uint32 NUM_WORK_GROUPS_Y = HEIGHT / 8;
