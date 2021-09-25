#include "defines.hpp"
// TODO: replace most C standard library calls with native platform layer
#include <stdio.h>
#include "pathtracer.hpp"
#include "loader.hpp"
#include <math.h>

int main()
{
	uint16 width = 400;
	uint16 height = 400;
	float aspectRatio = (float)width / (float)height;

	// Memory allocation for bitmap buffer
	uint8 *bitmapBuffer = (uint8 *)malloc(sizeof(uint8) * width * height * 3);

	// x is right, y is up, z is backwards
	Vec3f eye = CreateVec3f(0.0f, 0.0f, 0.0f);
	
	float gridHeight = 2.0f;
	float gridWidth = aspectRatio * gridHeight;
	Vec3f gridX = {gridWidth, 0.0f, 0.0f};
	Vec3f gridY = {0.0f, gridHeight, 0.0f};

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

	for(int16 ypixel = height - 1; ypixel >= 0; ypixel--)
	{
		for(int16 xpixel = 0; xpixel < width; xpixel++)
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
				color += EstimatorPathTracingLambertianNEE(ray, cornellBox);
			#else 
				color += EstimatorPathTracingLambertian(ray, cornellBox);
			#endif
			}

			// Divide by the number of sample rays sent through pixel
			// to get the average radiance accumulated
			color /= (float32)NUM_SAMPLES;

			// Gamma correction
			color.x = sqrtf(color.x);
			color.y = sqrtf(color.y);
			color.z = sqrtf(color.z);

			// TODO: why is it like this?
			int16 r = (int16)(255.99 * color.x);
			int16 g = (int16)(255.99 * color.y);
			int16 b = (int16)(255.99 * color.z);

			r = Clamp(r, 255);
			g = Clamp(g, 255);
			b = Clamp(b, 255);
			bitmapBuffer[(xpixel + (height - ypixel) * width) * 3 + 0] = (uint8)r;
			bitmapBuffer[(xpixel + (height - ypixel) * width) * 3 + 1] = (uint8)g;
			bitmapBuffer[(xpixel + (height - ypixel) * width) * 3 + 2] = (uint8)b;
		}
	}

	printf("Finished rendering to memory!\n");

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