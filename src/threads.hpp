#pragma once
#include "defines.hpp"
#include "intersect.hpp"

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
};

void *CreateThreadWin32(void *param);
void WaitForThreadWin32(void *threadHandle);
void CloseThreadWin32(void *threadHandle);
bool CanRelaunchThread(void *handle);