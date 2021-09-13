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

	uint16 width = 640;
	uint16 height = 360;
	float aspectRatio = (float)width / (float)height;

	// x is right, y is up, z is backwards
	Vec3f eye = {0.0f, 0.0f, 1.0f};
	
	float gridHeight = 2.0f;
	float gridWidth = aspectRatio * gridHeight;
	Vec3f gridX = {gridWidth, 0.0f, 0.0f};
	Vec3f gridY = {0.0f, gridHeight, 0.0f};

	// Lower left corner of virtual grid
	Vec3f gridOrigin = eye - (gridX / 2) - (gridY / 2);
	gridOrigin.z = 0.0f;

	fprintf(result, "P3\n%d %d\n255\n", width, height);

	// TODO: write to buffer first and then write to file
	for(int16 ypixel = height - 1; ypixel >= 0; ypixel--)
	{
		for(int16 xpixel = 0; xpixel < width; xpixel++)
		{
			float u = (float)xpixel / (float)width;
			float v = (float)ypixel / (float)height;
			Vec3f pointOnGrid = gridOrigin + u * gridX + v * gridY;
			Vec3f rayDirection = pointOnGrid - eye;
			Vec3f color = SkyColor(rayDirection);

			// TODO: why is it like this?
			int16 r = (int16)(255.99 * color.x);
			int16 g = (int16)(255.99 * color.y);
			int16 b = (int16)(255.99 * color.z);

			fprintf(result, "%d %d %d ", r, g, b);
		}
		fprintf(result, "\n");
	}

	fclose(result);
	printf("Finished rendering to image!\n");

	// TODO: timing
	return 0;
}