// TODO: replace most C standard library calls with native platform layer
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "loader.hpp"
#include "threads.hpp"
#include "scene/bvh.hpp"
#include "scene/material.hpp"
#include "scene/triangle.hpp"

int main()
{
	uint32 width = (uint32) WIDTH;
	uint32 height = (uint32) HEIGHT;
	float aspectRatio = (float)width / (float)height;
	
	// Memory allocation for bitmap buffer
	uint8 *bitmapBuffer = (uint8 *)malloc(sizeof(uint8) * width * height * 3);

	// x is right, y is up, z is backwards
	Vec3f eye = CreateVec3f(0.0f, 0.0f, 0.0f);
	
	float gridHeight = 2.0f;
	float gridWidth = aspectRatio * gridHeight;
	Vec3f gridX = {gridWidth, 0.0f, 0.0f};
	Vec3f gridY = {0.0f, -gridHeight, 0.0f}; // negative because of the way I am writing to the file

	// Lower left corner of virtual grid
	Vec3f gridOrigin = eye - (gridX / 2.0f) - (gridY / 2.0f);
	gridOrigin.z = -2.0f;

	Triangle *tris = nullptr;
	uint32 numTris = 0;

	Material **materials = nullptr;
	uint32 numMaterials = 0;

	if(!LoadModelFromObj("CornellBox-Suzanne.obj",
						 "../res/", 
						 &tris, &numTris,
						 &materials, &numMaterials))
	{
		return -1;
	}
	
	// Apply model matrix to tris
	Mat4f modelMatrix = CreateIdentityMat4f();
	modelMatrix = TranslationMat4f(CreateVec3f(0.0f, -1.0f, -3.5f), modelMatrix);
	for(uint32 i = 0; i < numTris; i++)
	{
		tris[i].v0 = modelMatrix * tris[i].v0;
		tris[i].v1 = modelMatrix * tris[i].v1;
		tris[i].v2 = modelMatrix * tris[i].v2;
		tris[i].edge1 = tris[i].v1 - tris[i].v0;
		tris[i].edge2 = tris[i].v2 - tris[i].v0;
	}

	// Construct BVH tree and sort triangle list according to it
	BVH_Node *rootBVH = nullptr;
	if(!ConstructBVH(tris, numTris, &rootBVH))
	{
		printf("Error in BVH construction!\n");
		return -1;
	}
	printf("Finished building BVH!\n");

	// Find all emissive triangles in scene
	uint32 *emissiveTris = new uint32[numTris];
	uint32 numEmissiveTris = 0;
	for(uint32 i = 0; i < numTris; i++)
	{
		if(tris[i].mat->Le.x >= 0.01f ||
		   tris[i].mat->Le.y >= 0.01f ||
		   tris[i].mat->Le.z >= 0.01f)
		{
			emissiveTris[numEmissiveTris++] = i;
		}
	}

	Scene scene = ConstructScene(nullptr, 0,
							     tris, numTris,
								 emissiveTris, numEmissiveTris,
								 rootBVH);

	// Each thread's handle and data to be used by it
	void* threadHandles[NUM_THREADS];
	RenderData* dataForThreads[NUM_THREADS];

	// Variables that keep track of the chunk data
	uint32 pixelStep = 64;
	uint32 rowIndex = 0, colIndex = 0;

	uint32 pixelSize = 3 * sizeof(uint8);
	uint32 rowSizeInBytes = pixelSize * width;
	uint32 threadMemorySize = pixelStep * pixelStep * pixelSize;

	uint16 numChunkColumns = (uint16)Ceil((float)width / (float)pixelStep);
	uint16 numChunkRows = (uint16)Ceil((float)height / (float)pixelStep);

	// If there are more threads than needed, don't allocate memory for them
	uint16 numThreads = Min((uint16)NUM_THREADS, numChunkRows * numChunkColumns);

	// Allocate memory for each one of the threads that will
	// be reused for all chunks to come
	for(uint16 i = 0; i < numThreads; i++)
	{
		uint8 *threadMemory = (uint8 *)malloc(threadMemorySize);

		dataForThreads[i] = (RenderData *)malloc(sizeof(RenderData));

		*dataForThreads[i] = 
		{
			(void *)threadMemory, 
			threadMemorySize,
			0, 0, 
			0, 0, 
			width, height,
			gridOrigin, gridX, gridY, eye,
			scene,
			false
		};
	}

	// Start a thread with the next chunk to be rendered
	for(uint16 i = 0; i <= numThreads; i++)
	{
		// Cycle around the thread array and check each thread infinitely
		if(i == numThreads)
			i = 0;

		// If we're at the end of the row, go down to the next row
		if(colIndex == numChunkColumns)
		{
			colIndex = 0;
			rowIndex++;
		}

		// We rendered all rows
		if(rowIndex == numChunkRows)
		{
			break;
		}

		// We can start a thread if it has never been started, OR
		// if the thread has started and finished its execution
		RenderData *data = dataForThreads[i];
#if defined(_WIN32) || defined(_WIN64)
		if(!data->initialized || CanThreadStartWin32(threadHandles[i]))
#elif defined(__linux__)
		if(!data->initialized || CanThreadStartLinux(threadHandles[i]))
#endif
		{
			// If initialized, this means that there was a previous run
			// that we can save to the bitmap buffer. If not, we need 
			// to initialize and start the thread for the first time
			if(data->initialized)
			{
				printf("Rendered region: x = %u-%u, y = %u-%u\n", data->startX, data->endX, data->startY, data->endY);
	
				int32 deltaX = (int32)(data->endX - data->startX);
				int32 deltaY = (int32)(data->endY - data->startY);

				// Read only this many bytes at a time from the thread's memory
				// and write it to the bitmap buffer. If we exceed this number 
				// of bytes, we will enter the next row and write bytes to it
				uint32 chunkRowSizeInBytes = pixelSize * deltaX;

				if(deltaX <= 0 || deltaY <= 0)
				{
					printf("Oh no we gotta go\n");
				}

				// Write the rendered region into the bitmap memory
				for(uint32 ymem = data->startY; ymem < data->endY; ymem++)
				{
					uint8 *offsetBitmapBuffer = bitmapBuffer + (ymem * rowSizeInBytes) + data->startX * pixelSize;
					memcpy(offsetBitmapBuffer, (uint8 *)data->threadMemoryChunk + (ymem - data->startY) * chunkRowSizeInBytes, chunkRowSizeInBytes);
				}
				
				// Close the finished thread because it can't be rerun
#if defined(_WIN32) || defined(_WIN64)
				CloseThreadWin32(threadHandles[i]);
#endif
			}

			// Set the thread's region parameters
			data->startX = colIndex * pixelStep;
			data->endX = Min(width, data->startX + pixelStep);
			data->startY = rowIndex * pixelStep;
			data->endY = Min(height, data->startY + pixelStep);
			
			// Make sure next run's rendered region is saved
			data->initialized = true;

			// Create another thread with the above data being the same
#if defined(_WIN32) || defined(_WIN64)
			threadHandles[i] = CreateThreadWin32(dataForThreads[i]);
#elif defined(__linux__)
			threadHandles[i] = CreateThreadLinux(dataForThreads[i]);
#endif

			// Increment column index
			colIndex++;
		}
	}

	// After we finish rendering all chunks
	for(uint16 i = 0; i < numThreads; i++)
	{
#if defined(_WIN32) || defined(_WIN64)
		// If the thread is currently running, wait for it (join it to main thread)
		if(!CanThreadStartWin32(threadHandles[i]))
			WaitForThreadWin32(threadHandles[i]);

		// Close the thread, we won't be needing it anymore
		CloseThreadWin32(threadHandles[i]);
#elif defined(__linux__)
		// If the thread is currently running, wait for it (join it to main thread)
		if(!CanThreadStartLinux(threadHandles[i]))
			WaitForThreadLinux(threadHandles[i]);
#endif

		// Thread just finished, so we need to write the changes
		RenderData *data = dataForThreads[i];
		
		printf("Rendered region: x = %u-%u, y = %u-%u\n", data->startX, data->endX, data->startY, data->endY);

		uint32 chunkRowSizeInBytes = pixelSize * (data->endX - data->startX);

		for(uint32 ymem = data->startY; ymem < data->endY; ymem++)
		{
			uint8 *offsetBitmapBuffer = bitmapBuffer + (ymem * rowSizeInBytes) + data->startX * pixelSize;
			memcpy(offsetBitmapBuffer, (uint8 *)data->threadMemoryChunk + (ymem - data->startY) * chunkRowSizeInBytes, chunkRowSizeInBytes);
		}

		// Free the allocated thread's memory, and the thread's data struct
		free(data->threadMemoryChunk);
		free(data);
	}

	// Write resulting rendered image to file
	FILE *result = fopen("../result.ppm", "w+");
	if(!result)
	{
		printf("Failed to create file!\n");
		return -1;
	}
	fprintf(result, "P3\n%d %d\n255\n", width, height);

	for(uint32 i = 0; i < (uint32)(width * height * 3); i+=3)
		fprintf(result, "%d %d %d\n", bitmapBuffer[i], bitmapBuffer[i+1], bitmapBuffer[i+2]);

	fclose(result);
	printf("Finished rendering to image!\n");

	free(bitmapBuffer);
	free(tris);
    for(uint32 i = 0; i < numMaterials; i++)
    {
        free(materials[i]);
    }

	free(materials);
    DeleteBVH(rootBVH);

	return 0;
}