#define BOUNCE_MIN 0
#define BOUNCE_COUNT 5
#define NUM_BOUNCES BOUNCE_MIN + BOUNCE_COUNT
#define NUM_SAMPLES 500
#define TMIN 0.0001f
#define TMAX 10000.0f
#define PI 3.14159265f
#define EPSILON 0.0001f
#define ENVIRONMENT_MAP_LE 0.0f

// TODO: replace most C standard library calls with native platform layer
#include <stdio.h>
#include "pathtracer.hpp"

int main()
{
	FILE *result = fopen("../result.ppm", "w+");
	if(!result)
	{
		printf("Failed to create file!\n");
		return -1;
	}

	uint16 width = 400;
	uint16 height = 400;
	float aspectRatio = (float)width / (float)height;

	// x is right, y is up, z is backwards
	Vec3f eye = CreateVec3f(0.0f, 0.0f, 2.0f);
	
	float gridHeight = 2.0f;
	float gridWidth = aspectRatio * gridHeight;
	Vec3f gridX = {gridWidth, 0.0f, 0.0f};
	Vec3f gridY = {0.0f, gridHeight, 0.0f};

	// Lower left corner of virtual grid
	Vec3f gridOrigin = eye - (gridX / 2.0f) - (gridY / 2.0f);
	gridOrigin.z = 0.0f;

	// CONSTRUCTING CORNELL BOX
	Scene cornellBox = {};
	
	Material cbMats[]
	{
		// light source
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.0f), CreateVec3f(20.0f)),
			
		// walls
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(1.0f, 0.1f, 0.1f), CreateVec3f(0.0f)),
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(0.1f, 1.0f, 0.1f), CreateVec3f(0.0f)),
		CreateMaterial(MATERIAL_LAMBERTIAN, CreateVec3f(1.0f, 1.0f, 1.0f), CreateVec3f(0.0f))
	};
	int32 cbNumMats = (int32)ARRAYCOUNT(cbMats);

	float32 cbOffset = -0.2f;
	float32 lightWidth = 0.5f;
	float32 lightYOffset = -0.01f;
	Quad cbQuads[]
	{
		// left
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -2.0f+cbOffset), 
				   CreateVec3f(-1.0f, 1.0f, 0.0f+cbOffset), 
				   CreateVec3f(1.0f, 0.0f, 0.0f), 
				   0, 1),
		// right
		CreateQuad(CreateVec3f(1.0f, -1.0f, -2.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, 0.0f+cbOffset), 
				   CreateVec3f(-1.0f, 0.0f, 0.0f), 
				   0, 2),
		// bottom
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -2.0f+cbOffset), 
				   CreateVec3f(1.0f, -1.0f, 0.0f+cbOffset), 
				   CreateVec3f(0.0f, 1.0f, 0.0f), 
				   1, 3),
		// top
		CreateQuad(CreateVec3f(-1.0f, 1.0f, -2.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, 0.0f+cbOffset), 
				   CreateVec3f(0.0f, -1.0f, 0.0f), 
				   1, 3),
		// back
		CreateQuad(CreateVec3f(-1.0f, -1.0f, -2.0f+cbOffset), 
				   CreateVec3f(1.0f, 1.0f, -2.0f+cbOffset), 
				   CreateVec3f(0.0f, 0.0f, 1.0f), 
				   2, 3),
			
		// light
		CreateQuad(CreateVec3f(-0.5f*lightWidth, 1.0f+lightYOffset, (-1.0f-0.5f*lightWidth)+cbOffset), 
			       CreateVec3f(0.5f*lightWidth, 1.0f+lightYOffset, (-1.0f+0.5f*lightWidth)+cbOffset), 
			       CreateVec3f(0.0f, -1.0f, 0.0f), 
				   1, 0),
	};
	int32 cbNumQuads = (int32)ARRAYCOUNT(cbQuads);

	Sphere cbSpheres[]
	{
		CreateSphere(CreateVec3f(0.5f, -0.7f, -1.0f), 0.3f, 3)
	};
	int32 cbNumSpheres = (int32)ARRAYCOUNT(cbSpheres);

	cornellBox = ConstructScene(cbSpheres, cbNumSpheres, cbQuads, cbNumQuads, cbMats, cbNumMats);

	// END CORNELL BOX
	
	fprintf(result, "P3\n%d %d\n255\n", width, height);

	// TODO: write to buffer first and then write to file
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
				// color += EstimatorPathTracingLambertian(ray, cornellBox);
				color += EstimatorPathTracingLambertianNEE(ray, cornellBox);
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

			fprintf(result, "%d %d %d ", r, g, b);
		}
		fprintf(result, "\n");
	}

	fclose(result);
	printf("Finished rendering to image!\n");

	// TODO: timing
	return 0;
}