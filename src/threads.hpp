#pragma once
#include "defines.hpp"
#include "scene/scene.hpp"

struct RenderData
{
	void *threadMemoryChunk;
	uint32 memorySize;

	// Virtual grid dimensions
	uint32 startX, endX;
	uint32 startY, endY;
	uint32 width, height;

	// Camera data
	Vec3f gridOrigin, gridX, gridY, eye;

	// Scene data
	Scene scene;

	// Flag to check if initialized
	bool initialized;
};

#if defined(_WIN32) || defined(_WIN64)
#include "threads_win32.hpp"
#endif