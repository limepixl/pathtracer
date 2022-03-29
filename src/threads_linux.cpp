#include <pthread.h>
#include <cstdio>
#include <errno.h>
#include "../thirdparty/pcg-c-basic-0.9/pcg_basic.h"
#include "../src/math/math.hpp"
#include "../src/pathtracer.hpp"

void *render_function(void *param)
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

	// Margins
	uint32 startX = renderData->startX;
	uint32 endX = renderData->endX;

	uint32 startY = renderData->startY;
	uint32 endY = renderData->endY;

	// Initialize a PCG context for each thread
	pcg32_random_t rng {};
	pcg32_srandom_r(&rng, time(NULL), (intptr_t)&rng);

	// Index for memory buffer
	uint32 index = 0;
	for(uint32 ypixel = renderData->startY; ypixel < endY; ypixel++)
	{
		uint32 y = (ypixel - renderData->startY);

		for(uint32 xpixel = renderData->startX; xpixel < endX; xpixel++)
		{
			uint32 x = (xpixel - renderData->startX);

			Vec3f color = {0.0f, 0.0f, 0.0f};

			for(int16 sample = 0; sample < NUM_SAMPLES; sample++)
			{
				// TODO: verify that this math checks out
				Vec2f offsetToPixelCenter = {0.5f, 0.5f};
				Vec2f uvOffset = RandomVec2fPCG(&rng) - offsetToPixelCenter;

				float u = ((float)xpixel + uvOffset.x) / (float)width;
				float v = ((float)ypixel + uvOffset.y) / (float)height;

				Vec3f pointOnGrid = gridOrigin + u * gridX + v * gridY;
				Vec3f rayDirection = NormalizeVec3f(pointOnGrid - eye);

				Ray ray = {eye, rayDirection};
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

	return NULL;
}

void *CreateThreadLinux(void *param)
{
	/*
	// Initialize thread attrib object
	pthread_attr_t threadAttrib;
	if(pthread_attr_init(&threadAttrib))
	{
		printf("Failed to initialize pthread_attr_t!\n");
		return NULL;
	}
	*/

	pthread_t *threadHandle = (pthread_t *)malloc(sizeof(pthread_t));
	if(pthread_create(threadHandle, NULL, &render_function, param))
	{
		printf("Failed to create pthread_t!\n");
		return NULL;
	}

	// printf("Successfully created thread with ID: %d\n", (int)threadID);
	return threadHandle;
}

void WaitForThreadLinux(void *threadHandle)
{
	pthread_join(*(pthread_t *)threadHandle, NULL);
	free(threadHandle);
}

bool CanThreadStartLinux(void *handle)
{
	if(pthread_tryjoin_np(*(pthread_t *)handle, NULL) == EBUSY)
		return false;

	return true;
}