// TODO: replace most C standard library calls with native platform layer
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.hpp"
#include "material.hpp"
#include "loader.hpp"
#include "threads.hpp"

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

	// Create and run threads
	void *threadHandles[NUM_THREADS];
	uint32 threadMemorySize = sizeof(uint8) * 3 * width * (height / NUM_THREADS);
	RenderData *dataForThreads[NUM_THREADS];
	for(uint8 i = 0; i < NUM_THREADS; i++)
	{
		// TODO: move malloc to the actual thread
		uint8 *threadMemory = (uint8 *)malloc(threadMemorySize);
		dataForThreads[i] = (RenderData *)malloc(sizeof(RenderData));
		
		*dataForThreads[i] = {(void *)threadMemory, threadMemorySize, 
							 (uint32)(i * (height / NUM_THREADS)), 
							 (uint32)((i+1) * (height / NUM_THREADS)), 
							 width, height,
							 gridOrigin, gridX, gridY, eye,
							 cornellBox};
		threadHandles[i] = CreateThreadWin32(dataForThreads[i]);
	}

	// Wait for threads to finish, close them and release their resources
	for(uint8 i = 0; i < NUM_THREADS; i++)
	{
		WaitForThreadWin32(threadHandles[i]);
		
		RenderData *currentData = dataForThreads[i];
		memcpy(bitmapBuffer + (i * currentData->memorySize), currentData->threadMemoryChunk, currentData->memorySize);
		free(currentData->threadMemoryChunk);
		free(currentData);

		// TODO: write currently rendered part into image
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
	return 0;
}