#pragma once

/*
	Defines
*/

#define NUM_BOUNCES 1
#define TMIN 0.0001
#define TMAX 10000.0

// NOTE: This only holds for LLP64
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

typedef float float32;
typedef double float64;

#include "vec3f.hpp"

/*
	Structs
*/

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

struct Sphere
{
	Vec3f origin;
	float32 radius;
};

struct HitData
{
	float32 t;
	Vec3f normal;
	Vec3f point;
};

#include "intersect.hpp"

/*
	Functions
*/

// Gets a point along the ray's direction vector.
// Assumes that the direction of the ray is normalized.
Vec3f PointAlongRay(Ray r, float32 t)
{
	return r.origin + r.direction * t;
}

// A simple lerp between 2 colors
Vec3f SkyColor(Vec3f dir)
{
	Vec3f normalizedDir = NormalizeVec3f(dir);
    float32 t = 0.5f*(normalizedDir.y + 1.0f);

	Vec3f startColor = {1.0f, 1.0f, 1.0f};
	Vec3f endColor = {0.546f, 0.824f, 0.925f};
    return (1.0f-t) * startColor + t * endColor;
}

bool Intersect(Ray ray, Sphere *spheres, int32 numSpheres, HitData *data)
{
	bool hitAnything = false;
	HitData resultData = {TMAX};
	for(int32 i = 0; i < numSpheres; i++)
	{
		HitData currentData = {};
		Sphere current = spheres[i];
		bool intersect = SphereIntersect(ray, current, &currentData);
		if(intersect)
		{
			hitAnything = true;

			if(currentData.t < resultData.t)
			{
				// Found closer hit, store it.
				resultData = currentData;
			}
		}
	}

	*data = resultData;
	return hitAnything;
}

// Cast ray into the scene, have the ray bounce around
// a predetermined number of times accumulating radiance
// along the way, and return the color for the given pixel
Vec3f CastRay(Ray ray, uint8 numBounces, Sphere *spheres, int32 numSpheres)
{
	// TODO: for now there are only spheres, but we need a formal
	// definition of a scene with any objects within it!

	Vec3f color {0.0f, 0.0f, 0.0f};

	for(uint8 b = 0; b < numBounces; b++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, spheres, numSpheres, &data);
		if(intersect) // determine color and bounce
		{
			Vec3f offset = {1.0f, 1.0f, 1.0f};
			color = 0.5f * (data.normal + offset);
		}
		else // add background illumination and break
		{
			color += SkyColor(ray.direction);
		}
	}

	return color;
}