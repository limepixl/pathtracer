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
	uint32 width = 1920;
	uint32 height = 1080;
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

	// CONSTRUCTING CORNELL BOX
	Scene cornellBox = {};
	
	Material cbMats[]
	{
		// light source
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.0f), CreateVec3f(20.0f)),
			
		// walls
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(1.0f, 0.1f, 0.1f), CreateVec3f(0.0f)),
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.1f, 1.0f, 0.1f), CreateVec3f(0.0f)),
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(1.0f, 1.0f, 1.0f), CreateVec3f(0.0f)),

		// alternative walls
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.98f, 0.52f, 0.1f), CreateVec3f(0.0f)),
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.1f, 0.62f, 0.98f), CreateVec3f(0.0f)),

	};
	int32 cbNumMats = (int32)ARRAYCOUNT(cbMats);

	float32 cbOffset = -0.05f;
	float32 lightWidth = 0.5f;
	float32 lightYOffset = -0.01f;
	float32 lightXOffset = 0.3f;
	Quad cbQuads[]
	{
		// left
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -4.0f+cbOffset), 
				   CreateVec3f(-1.0f, 1.0f, -2.0f+cbOffset), 
				   CreateVec3f(1.0f, 0.0f, 0.0f), 
				   0, 4),
		// right
		CreateQuad(CreateVec3f(1.0f, -1.0f, -4.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, -2.0f+cbOffset), 
				   CreateVec3f(-1.0f, 0.0f, 0.0f), 
				   0, 5),
		// bottom
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -4.0f+cbOffset), 
				   CreateVec3f(1.0f, -1.0f, -2.0f+cbOffset), 
				   CreateVec3f(0.0f, 1.0f, 0.0f), 
				   1, 3),
		// top
		CreateQuad(CreateVec3f(-1.0f, 1.0f, -4.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, -2.0f+cbOffset), 
				   CreateVec3f(0.0f, -1.0f, 0.0f), 
				   1, 3),
		// back
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -4.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, -4.0f+cbOffset), 
				   CreateVec3f(0.0f, 0.0f, 1.0f), 
				   2, 3),
		
		// light
		CreateQuad(CreateVec3f(-0.5f*lightWidth, 1.0f+lightYOffset, (-3.0f-0.5f*lightWidth)+cbOffset), 
			       CreateVec3f(0.5f*lightWidth, 1.0f+lightYOffset, (-3.0f+0.5f*lightWidth)+cbOffset), 
			       CreateVec3f(0.0f, -1.0f, 0.0f), 
				   1, 0),
	};
	int32 cbNumQuads = (int32)ARRAYCOUNT(cbQuads);

	LightSource cbLights[]
	{
		CreateLightSource(&cbQuads[5], LightSourceType::QUAD),
	};
	int32 cbNumLights = (int32)ARRAYCOUNT(cbLights);

	// Load OBJ model
	Triangle *modelTris;
	int32 numTris;
	bool loadedOBJ = LoadModelFromObj("../res/suzanne.obj", &modelTris, &numTris);

	// Model transformation matrix
	Mat4f modelMatrix = CreateIdentityMat4f();
	modelMatrix = ScaleMat4f(CreateVec3f(0.3f, 0.3f, 0.3f), modelMatrix);
	modelMatrix = TranslationMat4f(CreateVec3f(0.0f, 0.0f, -3.0f), modelMatrix);

	TriangleModel triModels[]
	{
		CreateTriangleModel(modelTris, numTris, modelMatrix, 3)
	};
	int32 numTriModels = (int32)ARRAYCOUNT(triModels);

	cornellBox = ConstructScene(NULL, 0, 
							    cbQuads, cbNumQuads,
								triModels, numTriModels,
								cbLights, cbNumLights,
								cbMats, cbNumMats);

	// END CORNELL BOX

	// Each thread's handle and data to be used by it
	void *threadHandles[NUM_THREADS];
	RenderData *dataForThreads[NUM_THREADS];

	// y keeps track of what row is next to be rendered
	uint32 x = 0;
	uint32 y = 0;
	uint32 pixelStep = 64;

	uint32 pixelSize = 3 * sizeof(uint8);
	uint32 rowSizeInBytes = pixelSize * width;

	uint32 numChunkColumns = (uint32)Ceil((float32)width / pixelStep);
	uint32 numChunkRows = (uint32)Ceil((float32)height / pixelStep);
	uint32 rowIndex = 0, colIndex = 0;

	uint8 runningThreads = NUM_THREADS;
	for(uint8 i = 0; i < NUM_THREADS; i++)
	{
		// If we're at the end of the row, go down to the next row
		if(colIndex == numChunkColumns)
		{
			colIndex = 0;
			rowIndex++;
		}

		if(rowIndex == numChunkRows)
		{
			runningThreads = i;
			break;
		}

		// X
		uint32 startX = colIndex * pixelStep;
		uint32 endX = (colIndex + 1) * pixelStep;
		endX = Min(endX, width);

		int32 deltaX = endX - startX;

		// Y
		uint32 startY = rowIndex * pixelStep;
		uint32 endY = (rowIndex + 1) * pixelStep;
		endY = Min(endY, height);

		int32 deltaY = endY - startY;

		if(deltaX <= 0 || deltaY <= 0)
		{
			printf("Oh no we gotta go\n");
		}

		uint32 chunkRowSizeInBytes = pixelSize * deltaX;
		uint32 threadMemorySize = pixelStep * pixelStep * pixelSize;

		uint8 *threadMemory = (uint8 *)malloc(threadMemorySize);
		
		// Go to next column in row
		colIndex++;

		// Allocate and initialize memory for each thread's data
		dataForThreads[i] = (RenderData *)malloc(sizeof(RenderData));
		*dataForThreads[i] = {(void *)threadMemory, 
							 threadMemorySize,
							 startX, endX, 
							 startY, endY, 
							 width, height,
							 gridOrigin, gridX, gridY, eye,
							 cornellBox};

		// Create and start up each thread
		threadHandles[i] = CreateThreadWin32(dataForThreads[i]);
	}

	// Start a thread with the next chunk to be rendered
	for(uint8 i = 0; i <= runningThreads; i++)
	{
		// If we're at the end of the row, go down to the next row
		if(colIndex == numChunkColumns)
		{
			colIndex = 0;
			rowIndex++;
		}

		// We rendered all rows
		if(rowIndex == numChunkRows)
			break;

		// Cycle threads
		if(i == runningThreads)
			i = 0;

		// If we can relaunch the thread, launch it to render the next row
		if(CanRelaunchThread(threadHandles[i]))
		{
			RenderData *data = dataForThreads[i];
			
			// X
			uint32 startX = data->startX;
			uint32 endX = data->endX;
			endX = Min(endX, width);

			int32 deltaX = endX - startX;

			// Y
			uint32 startY = data->startY;
			uint32 endY = data->endY;
			endY = Min(endY, height);

			int32 deltaY = endY - startY;

			uint32 chunkRowSizeInBytes = pixelSize * deltaX;

			if(deltaX <= 0 || deltaY <= 0)
			{
				printf("Oh no we gotta go\n");
			}

			// Write the rendered region into the bitmap memory
			for(uint32 ymem = data->startY; ymem < endY; ymem++)
			{
				uint8 *offsetBitmapBuffer = bitmapBuffer + (ymem * rowSizeInBytes) + data->startX * pixelSize;
				memcpy(offsetBitmapBuffer, (uint8 *)data->threadMemoryChunk + (ymem - data->startY) * chunkRowSizeInBytes, chunkRowSizeInBytes);
			}

			data->startX = colIndex * pixelStep;
			data->endX = Min(width, (colIndex + 1) * pixelStep);
			data->startY = rowIndex * pixelStep;
			data->endY = Min(height, (rowIndex + 1) * pixelStep);

			// Increment column index
			colIndex++;
			
			// Close the finished thread and create a new one with
			// the new data to be executed
			CloseThreadWin32(threadHandles[i]);
			threadHandles[i] = CreateThreadWin32(dataForThreads[i]);
		}
	}

	// After we finish rendering all chunks
	for(uint8 i = 0; i < runningThreads; i++)
	{
		// If the thread is currently running, wait for it (join it to main thread)
		if(!CanRelaunchThread(threadHandles[i]))
			WaitForThreadWin32(threadHandles[i]);

		// Close the thread, we won't be needing it anymore
		CloseThreadWin32(threadHandles[i]);
		
		// Thread just finished, so we need to write the changes
		RenderData *data = dataForThreads[i];
		
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

	return 0;
}