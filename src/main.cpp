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

	uint16 width = 1280;
	uint16 height = 720;
	float aspectRatio = (float)width / (float)height;

	// x is right, y is up, z is backwards
	Vec3f eye = {0.0f, 0.0f, 1.0f};
	
	float gridHeight = 2.0f;
	float gridWidth = aspectRatio * gridHeight;
	Vec3f gridX = {gridWidth, 0.0f, 0.0f};
	Vec3f gridY = {0.0f, gridHeight, 0.0f};

	// Lower left corner of virtual grid
	Vec3f gridOrigin = eye - (gridX / 2.0f) - (gridY / 2.0f);
	gridOrigin.z = 0.0f;

	Material materials[]
	{
		CreateMaterial(CreateVec3f(0.8f, 0.1f, 0.0f) / PI, CreateVec3f(0.8f, 0.1f, 0.0f)),
		CreateMaterial(CreateVec3f(0.8f, 0.8f, 0.8f) / PI, CreateVec3f(0.0f, 0.0f, 0.0f))
	};
	int32 numMaterials = (int32)(sizeof(materials) / sizeof(Material));

	Sphere spheres[]
	{
		{ {0.0f, 0.0f, -2.0f}, 1.0f, &materials[0] },
		{ {0.0f, -101.0f, -2.0f}, 100.0f, &materials[1] }
	};
	int32 numSpheres = (int32)(sizeof(spheres) / sizeof(Sphere));

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
				color += EstimatorPathTracingLambertian(ray, NUM_BOUNCES, spheres, numSpheres);
			}

			// Divide by the number of sample rays sent through pixel
			// to get the average radiance accumulated
			color /= (float)NUM_SAMPLES;

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