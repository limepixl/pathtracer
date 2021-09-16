#pragma once

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

// Macro to return stack allocated array length
#define ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#include "math.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

// Gets a point along the ray's direction vector.
// Assumes that the direction of the ray is normalized.
Vec3f PointAlongRay(Ray r, float32 t)
{
	return r.origin + r.direction * t;
}

#include "intersect.hpp"
#include "material.hpp"

// A simple lerp between 2 colors
Vec3f SkyColor(Vec3f dir)
{
	Vec3f normalizedDir = NormalizeVec3f(dir);
    float32 t = 0.5f*(normalizedDir.y + 1.0f);

	Vec3f startColor = {1.0f, 1.0f, 1.0f};
	Vec3f endColor = {0.546f, 0.824f, 0.925f};
    return (1.0f-t) * startColor + t * endColor;
}

bool VisibilityTerm(Ray shadowRay, Scene scene, Quad *lightSource, Vec3f *normalY)
{
	HitData data = {};
	bool hitAnything = Intersect(shadowRay, scene, &data);
	if(hitAnything && data.materialIndex == 0)
	{
		// We hit the light source!
		*normalY = data.normal;
		return true;
	}

	return false;
}

float32 GeometryTerm(Vec3f x, Vec3f y, Vec3f normalX, Vec3f normalY, Ray ray)
{
	Vec3f distVector = y-x;
	float32 distanceSquared = Dot(distVector, distVector);
	
	// Only account for the upper hemisphere
	float32 cosTerm1 = Max(0.0f, Dot(normalX,  ray.direction));

	// Account for both sides of the light source
	float32 cosTerm2 = Abs(Dot(normalY, ray.direction));

	return (cosTerm1 * cosTerm2) / distanceSquared;
}

Vec3f DirectIllumination(Scene scene, Vec3f BRDF, Vec3f throughputTerm, HitData *data)
{
	// direct illumination by light sources in the scene
	// (next event estimation / shadow rays sent out to light sources)
	Vec3f directIllumination = CreateVec3f(0.0f);
	for(int16 shadowRayIndex = 0; shadowRayIndex < NUM_SHADOWRAYS; shadowRayIndex++)
	{
		Quad *lightSource = scene.lights[0];

		// Sample random point on light source
		Vec2f uv = RandomVec2f();
		float32 u = uv.x;
		float32 v = uv.y;
		Vec3f quadDims = lightSource->end - lightSource->origin;
		float32 quadArea = 0.0f;

		Vec3f pointOnLight = lightSource->origin;
		if(lightSource->component == 0)
		{
			quadArea = quadDims.y * quadDims.z;
			pointOnLight.y += quadDims.y * u;
			pointOnLight.z += quadDims.z * v;
		}
		else if(lightSource->component == 1)
		{
			quadArea = quadDims.x * quadDims.z;
			pointOnLight.x += quadDims.x * u;
			pointOnLight.z += quadDims.z * v;
		}
		else
		{
			quadArea = quadDims.x * quadDims.y;
			pointOnLight.x += quadDims.x * u;
			pointOnLight.x += quadDims.y * v;
		}

		Material *lightSourceMat = &scene.materials[lightSource->materialIndex];

		Vec3f x = data->point;
		Vec3f y = pointOnLight;

		Ray shadowRay = {x, NormalizeVec3f(y-x)};

		Vec3f normalX = data->normal;
		Vec3f normalY = CreateVec3f(0.0f);

		if(VisibilityTerm(shadowRay, scene, lightSource, &normalY))
		{
			directIllumination += lightSourceMat->Le * BRDF * throughputTerm *
								  GeometryTerm(x, y, normalX, normalY, shadowRay);

			// Divide the illumination by the PDF of the light sampling
			float32 NEEPDF = 1.0f / quadArea;
			directIllumination /= NEEPDF;
		}
	}

	directIllumination /= (float32)NUM_SHADOWRAYS;

	return directIllumination;
}

// Cast ray into the scene, have the ray bounce around
// a predetermined number of times accumulating radiance
// along the way, and return the color for the given pixel
Vec3f EstimatorPathTracingLambertian(Ray ray, Scene scene)
{
	Vec3f color {0.0f, 0.0f, 0.0f};

	// ( BRDF * dot(Nx, psi) ) / PDF(psi)
	Vec3f throughputTerm {1.0f, 1.0f, 1.0f};

	// Vignette effect (basically undoing We=1/cos^3(theta) )
	// float32 theta = acosf(-ray.direction.z);
	// throughputTerm = throughputTerm * powf(cosf(theta), 3);

	int8 numBounces = (int8)Max(5, NUM_BOUNCES);
	for(uint8 b = 0; b < numBounces; b++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if(!intersect) // ray goes off into infinity
		{
			if(BOUNCE_MIN <= b && b <= NUM_BOUNCES)
				color += throughputTerm * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Material *mat = &scene.materials[data.materialIndex];
		float32 cosTheta = Dot(ray.direction, NormalizeVec3f(data.normal));

		// add the light that the material emits
		if(BOUNCE_MIN <= b && b <= NUM_BOUNCES)
		{
			color += throughputTerm * mat->Le;

			#if NEE
			if(mat->Le.x > 0.05f)
				break;
			#endif 
		}

		#if NEE
		// If hit non-light point (TEMP)
		if(mat->Le.x < 0.05f)
		{
			Vec3f directIllumination = 
				DirectIllumination(scene, mat->color, throughputTerm, &data);
			color += throughputTerm * directIllumination;
		}
		#endif

		// update throughput
		// The PI is here because we are sampling w.r.t the pdf
		// p(psi) = cos(theta) / PI.
		// This cosine term cancels out with the dot product in
		// the throughput term and all that is left is the BRDF
		// along with PI.
		throughputTerm *= PI * mat->color;

		// pdf(psi) = cos(theta) / PI
		Vec2f randomVec2f = RandomVec2f();
		Vec3f dir = MapToUnitHemisphereCosineWeighted(randomVec2f, data.normal);

		Vec3f point = data.point;
		ray = {point + EPSILON * data.normal, dir};
	}

	return color;
}