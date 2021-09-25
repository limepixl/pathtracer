#include "threads.hpp"
#include <Windows.h>
#include <stdio.h>
#include "pathtracer.hpp"

DWORD WINAPI render_function(LPVOID param)
{
	RenderData *renderData = (RenderData *)param;

	uint32 width = renderData->width;
	uint32 height = renderData->height;

	Vec3f gridOrigin = renderData->gridOrigin;
	Vec3f gridX = renderData->gridX;
	Vec3f gridY = renderData->gridY;
	Vec3f eye = renderData->eye;

	Scene scene = renderData->scene;

	uint8 *memory = (uint8 *)renderData->threadMemoryChunk;

	uint32 deltaY = renderData->endY - renderData->startY;
	for(int32 ypixel = renderData->startY; ypixel < (int32)renderData->endY; ypixel++)
	{
		uint32 y = (ypixel - renderData->startY);

		for(int32 xpixel = 0; xpixel < (int32)width; xpixel++)
		{
			Vec3f color = {0.0f, 0.0f, 0.0f};

			for(int16 sample = 0; sample < NUM_SAMPLES; sample++)
			{
				Vec2f offsetToPixelCenter = {0.5f, 0.5f};
				Vec2f uvOffset = RandomVec2f() - offsetToPixelCenter;

				float u = ((float)xpixel + uvOffset.x) / (float)width;
				float v = ((float)ypixel + uvOffset.y) / (float)height;

				Vec3f pointOnGrid = gridOrigin + u * gridX + v * gridY;
				Vec3f rayDirection = NormalizeVec3f(pointOnGrid - eye);

				Ray ray = {eye, rayDirection};
			#if NEE_ONLY
				color += EstimatorPathTracingLambertianNEE(ray, scene);
			#else 
				color += EstimatorPathTracingLambertian(ray, scene);
			#endif
			}

			// Divide by the number of sample rays sent through pixel
			// to get the average radiance accumulated
			color /= (float32)NUM_SAMPLES;

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

			
			memory[(xpixel + y * width) * 3 + 0] = (uint8)r;
			memory[(xpixel + y * width) * 3 + 1] = (uint8)g;
			memory[(xpixel + y * width) * 3 + 2] = (uint8)b;
		}
		// printf("THREAD: %d-%d | Progress: %.2f%%\n", renderData->startY, renderData->endY, (float32)y / deltaY);
	}

	// printf("Finished rendering from y=%d to y=%d\n", renderData->startY, renderData->endY);
	ExitThread(0);
}

void *CreateThreadWin32(void *param)
{
	DWORD threadID = {};
	HANDLE threadHandle = CreateThread(NULL, 0, render_function, param, 0, &threadID);
	if(threadID == NULL)
	{
		OutputDebugStringA("Failed to create thread!\n");
	}

	// printf("Successfully created thread with ID: %d\n", (int)threadID);
	return threadHandle;
}

void WaitForThreadWin32(void *threadHandle)
{
	WaitForSingleObject(threadHandle, INFINITE);
	CloseHandle(threadHandle);
}

bool CanRelaunchThread(void *handle)
{
	DWORD exitCode = {};
	BOOL res = GetExitCodeThread(handle, &exitCode);
	if(exitCode == STILL_ACTIVE)
		return false;

	return true;
}