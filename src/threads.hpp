#pragma once
#include "defines.hpp"
#include "scene/scene.hpp"

struct RenderData
{
	void *thread_memory_chunk;
	uint32 memory_size;

	// Virtual grid dimensions
	uint32 start_x, end_x;
	uint32 start_y, end_y;
	uint32 width, height;

	// Camera data
	Vec3f grid_origin, grid_x, grid_y, eye;

	// Scene data
	Scene scene;

	// Flag to check if initialized
	bool initialized;
};

#if defined(_WIN32) || defined(_WIN64)
#include "threads_win32.hpp"
#endif

#ifdef __linux__
#include "threads_linux.hpp"
#endif
