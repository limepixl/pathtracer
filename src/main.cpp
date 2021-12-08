// TODO: replace most C standard library calls with native platform layer
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.hpp"
#include "material.hpp"
#include "loader.hpp"
#include "threads.hpp"
#include "math.hpp"

int main()
{
	uint32 width = (uint32) WIDTH;
	uint32 height = (uint32) HEIGHT;
	float32 aspectRatio = (float32)width / (float32)height;
	
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

	uint32 *cbEmissiveTris = NULL;
	uint32 numCbEmissiveTris = 0;

	Triangle *cbTris = NULL;
	uint32 numCbTris = 0;

	Material *materials = NULL;
	uint32 numMaterials = 0;

	bool loadedCornellBox = LoadModelFromObj("CornellBox-Suzanne.obj",
											 "../res/", 
											 &cbTris, &numCbTris,
											 &cbEmissiveTris, &numCbEmissiveTris,
											 &materials, &numMaterials);

	if(!loadedCornellBox)
		return -1;

	Mat4f modelMatrix = CreateIdentityMat4f();
	modelMatrix = TranslationMat4f(CreateVec3f(0.0f, -1.0f, -3.5f), modelMatrix);

	TriangleModel triModels[]
	{
		CreateTriangleModel(cbTris, numCbTris, modelMatrix)
	};
	uint32 numTriModels = (uint32)ARRAYCOUNT(triModels);

	Scene cornellBox = ConstructScene(NULL, 0,
									  NULL, 0,
									  triModels, numTriModels,
									  cbEmissiveTris, numCbEmissiveTris);

	// Each thread's handle and data to be used by it
	void* threadHandles[NUM_THREADS];
	RenderData* dataForThreads[NUM_THREADS];

	// Variables that keep track of the chunk data
	uint32 pixelStep = 64;
	uint32 rowIndex = 0, colIndex = 0;

	uint32 pixelSize = 3 * sizeof(uint8);
	uint32 rowSizeInBytes = pixelSize * width;
	uint32 threadMemorySize = pixelStep * pixelStep * pixelSize;

	uint16 numChunkColumns = (uint16)Ceil((float32)width / pixelStep);
	uint16 numChunkRows = (uint16)Ceil((float32)height / pixelStep);

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
			cornellBox,
			false
		};
	}

	// Start a thread with the next chunk to be rendered
	for(uint8 i = 0; i <= numThreads; i++)
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
		if(!data->initialized || CanThreadStart(threadHandles[i]))
		{
			// If initialized, this means that there was a previous run
			// that we can save to the bitmap buffer. If not, we need 
			// to initialize and start the thread for the first time
			if(data->initialized)
			{
				printf("Rendered region: x = %u-%u, y = %u-%u\n", data->startX, data->endX, data->startY, data->endY);
	
				int32 deltaX = data->endX - data->startX;
				int32 deltaY = data->endY - data->startY;

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
				CloseThreadWin32(threadHandles[i]);
			}

			// Set the thread's region parameters
			data->startX = colIndex * pixelStep;
			data->endX = Min(width, data->startX + pixelStep);
			data->startY = rowIndex * pixelStep;
			data->endY = Min(height, data->startY + pixelStep);
			
			// Make sure next run's rendered region is saved
			data->initialized = true;

			// Create another thread with the above data being the same
			threadHandles[i] = CreateThreadWin32(dataForThreads[i]);

			// Increment column index
			colIndex++;
		}
	}

	// After we finish rendering all chunks
	for(uint8 i = 0; i < numThreads; i++)
	{
		// If the thread is currently running, wait for it (join it to main thread)
		if(!CanThreadStart(threadHandles[i]))
			WaitForThreadWin32(threadHandles[i]);

		// Close the thread, we won't be needing it anymore
		CloseThreadWin32(threadHandles[i]);

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
	free(cbTris);
	free(materials);

	return 0;
}