#include "pathtracer.hpp"
#include <Windows.h>
#include <ctime>

DWORD WINAPI render_function(LPVOID param)
{
	RenderData *render_data = (RenderData *)param;

	uint32 width = render_data->width;
	uint32 height = render_data->height;

	Vec3f grid_origin = render_data->grid_origin;
	Vec3f grid_x = render_data->grid_x;
	Vec3f grid_y = render_data->grid_y;
	Vec3f eye = render_data->eye;

	Scene scene = render_data->scene;

	uint8 *memory = (uint8 *)render_data->thread_memory_chunk;

	// Margins
	uint32 start_x = render_data->start_x;
	uint32 end_x = render_data->end_x;

	uint32 start_y = render_data->start_y;
	uint32 end_y = render_data->end_y;

	// Initialize a PCG context for each thread
	pcg32_random_t rng {};
#if PREDICTABLE_RAND
	pcg32_srandom_r(&rng, 0, (intptr_t)&rng);
#else
	pcg32_srandom_r(&rng, time(nullptr), (intptr_t)&rng);
#endif

	// Index for memory buffer
	uint32 index = 0;
	for (uint32 ypixel = render_data->start_y; ypixel < end_y; ypixel++)
	{
		uint32 y = (ypixel - render_data->start_y);

		for (uint32 xpixel = render_data->start_x; xpixel < end_x; xpixel++)
		{
			uint32 x = (xpixel - render_data->start_x);

			Vec3f color = { 0.0f, 0.0f, 0.0f };

			for (int16 sample = 0; sample < NUM_SAMPLES; sample++)
			{
				// TODO: verify that this math checks out
				Vec2f offset_to_pixel_center = { 0.5f, 0.5f };
				Vec2f uv_offset = RandomVec2fPCG(&rng) - offset_to_pixel_center;

				float u = ((float)xpixel + uv_offset.x) / (float)width;
				float v = ((float)ypixel + uv_offset.y) / (float)height;

				Vec3f point_on_grid = grid_origin + u * grid_x + v * grid_y;
				Vec3f ray_direction = NormalizeVec3f(point_on_grid - eye);

				Ray ray = { eye, ray_direction, 1.0f / ray_direction };
				// color += EstimatorPathTracingLambertian(ray, scene, &rng);
				// color += EstimatorPathTracingLambertianNEE(ray, scene, &rng);
				color += EstimatorPathTracingMIS(ray, scene, &rng);
			}

			// Divide by the number of sample rays sent through pixel
			// to get the average radiance accumulated
			color /= (float)NUM_SAMPLES;

			// Gamma correction
			color.x = sqrtf(color.x);
			color.y = sqrtf(color.y);
			color.z = sqrtf(color.z);

			int16 r = (int16)(255.99 * color.x);
			int16 g = (int16)(255.99 * color.y);
			int16 b = (int16)(255.99 * color.z);

			r = Clamp(r, 255);
			g = Clamp(g, 255);
			b = Clamp(b, 255);

			memory[3 * index + 0] = (uint8)r;
			memory[3 * index + 1] = (uint8)g;
			memory[3 * index + 2] = (uint8)b;
			index++;
		}
	}

	return 1;
}

void *CreateThreadWin32(void *param)
{
	DWORD threadID = {};
	HANDLE thread_handle = CreateThread(nullptr, 0, render_function, param, 0, &threadID);
	if (threadID == 0)
	{
		OutputDebugStringA("Failed to create thread!\n");
	}

	// printf("Successfully created thread with ID: %d\n", (int)threadID);
	return thread_handle;
}

void CloseThreadWin32(void *thread_handle)
{
	CloseHandle(thread_handle);
}

void WaitForThreadWin32(void *thread_handle)
{
	WaitForSingleObject(thread_handle, INFINITE);
}

bool CanThreadStartWin32(void *handle)
{
	DWORD exit_code = {};
	BOOL res = GetExitCodeThread(handle, &exit_code);
	if (exit_code == STILL_ACTIVE)
		return false;

	return true;
}