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

// Cast ray into the scene, have the ray bounce around
// a predetermined number of times accumulating radiance
// along the way, and return the color for the given pixel
Vec3f EstimatorPathTracingLambertian(Ray ray, Scene scene)
{
	Vec3f color {0.0f, 0.0f, 0.0f};

	// ( BRDF * dot(Nx, psi) ) / PDF(psi)
	Vec3f throughputTerm {1.0f, 1.0f, 1.0f};

	Vec3f oldNormal = CreateVec3f(0.0f);

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
			color += throughputTerm * mat->Le;

		// update throughput
		// The PI is here because we are sampling w.r.t the pdf
		// p(psi) = cos(theta) / PI.
		// This cosine term cancels out with the dot product in
		// the throughput term and all that is left is the BRDF
		// along with PI.
		throughputTerm *= PI * mat->color;

		Vec2f randomVec2f = RandomVec2f();

		// pdf(psi) = cos(theta) / PI
		Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, data.normal);

		Vec3f point = data.point;
		ray = {point + EPSILON * data.normal, dir};

		oldNormal = data.normal;
	}

	return color;
}

Vec3f EstimatorPathTracingLambertianNEE(Ray ray, Scene scene)
{
	Vec3f color = CreateVec3f(0.0f);
	Vec3f throughputTerm = CreateVec3f(1.0f);

	for(int16 bounce = 0; bounce < NUM_BOUNCES; bounce++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if(!intersect)
		{
			// No intersection with scene, add env map contribution
			color += throughputTerm * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Vec3f &BRDF = scene.materials[data.materialIndex].color; // convenience
		Material *mat = &scene.materials[data.materialIndex];
		float32 cosTheta = Dot(data.normal, ray.direction);
		float32 pdfCosineWeightedHemisphere = cosTheta / PI;

		// add light that is emitted from surface (but bounce right afterwards)
		bool hitLight = false;
		if(BOUNCE_MIN <= bounce && bounce <= NUM_BOUNCES && mat->Le.x >= 0.5f)
		{
			hitLight = true;
			if(bounce == 0)
				color = throughputTerm * mat->Le;
			break;
		}
		// If there is at least 1 light source in the scene and the material
		// of the surface we hit is diffuse, we can use NEE.
		else if(scene.numLightSources > 0 && mat->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			// sample light sources for direct illumination
			Vec3f directIllumination = CreateVec3f(0.0f);
			for(int8 shadowRayIndex = 0; shadowRayIndex < NUM_SHADOW_RAYS; shadowRayIndex++)
			{
				// pick a light source
				// TEMP: we only have one light source
				// TODO: add a way to have a separate array of light sources
				float32 pdfPickLight = 1.0f / scene.numLightSources;

				int32 pickedLightSource = (int32)(rand() % scene.numLightSources);
				LightSource lightSource = scene.lightSources[pickedLightSource];

				
				Material *lightSourceMat = NULL;
				Vec3f y = CreateVec3f(0.0f);
				float32 lightArea = 0.0f;
				if(lightSource.type == LightSourceType::QUAD)
				{
					Quad *quadLight = (Quad *)(lightSource.obj);
					lightSourceMat = &scene.materials[quadLight->materialIndex];

					Vec3f v0 = quadLight->origin;
					Vec3f v2 = quadLight->end;
					Vec3f dims = v2 - v0;

					// Get the area of the light source
					float32 quadArea = 0.0f;
					if(quadLight->component == 0) // yz
						quadArea = dims.y * dims.z;
					else if(quadLight->component == 1) // xz
						quadArea = dims.x * dims.z;
					else // xy
						quadArea = dims.x * dims.y;

					lightArea = quadArea;

					// Pick y on the light
					Vec2f uv = RandomVec2f();
					Vec3f pointOnLight = quadLight->origin;
					if(quadLight->component == 0) // x
					{
						pointOnLight.y += uv.x * dims.y;
						pointOnLight.z += uv.y * dims.z;
					}
					else if(quadLight->component == 1) // y
					{
						pointOnLight.x += uv.x * dims.x;
						pointOnLight.z += uv.y * dims.z;
					}
					else // z
					{
						pointOnLight.x += uv.x * dims.x;
						pointOnLight.y += uv.y * dims.y;
					}
					y = pointOnLight;
				}
				float32 pdfPickPointOnLight = 1.0f / lightArea;

				// PDF in terms of area, for picking point on light source k
				float32 pdfLight_area = pdfPickLight * pdfPickPointOnLight;

				// Just for clarity
				Vec3f x = data.point;

				// Send out a shadow ray in direction x->y
				Vec3f distVec = y - x;
				Vec3f shadowRayDir = NormalizeVec3f(distVec);
				Ray shadowRay = {x, shadowRayDir};

				// Check if ray hits anything before hitting the light source
				HitData shadowData = {};
				bool hitAnything = Intersect(shadowRay, scene, &shadowData);
				if(!hitAnything)
				{
					// It shouldn't be possible to send a ray towards the light
					// source and not hit anything in the scene, even the light
					printf("ERROR: Shadow ray didn't hit anything!\n");
					break;
				}

				// Visibility check means we have a clear line of sight!
				if(y == shadowData.point)
				{
					float32 squaredDist = Dot(distVec, distVec);

					// We want to only add light contribution from lights within 
					// the hemisphere solid angle above X, and not from lights behind it.
					float32 cosThetaX = Max(0.0f, Dot(data.normal, shadowRayDir));

					// We can sample the light from both sides, it doesn't have to
					// be a one-sided light source.
				#if TWOSIDED_LIGHT_QUADS
					float32 cosThetaY = Abs(Dot(shadowData.normal, shadowRayDir));
				#else
					float32 cosThetaY = Max(0.0f, Dot(shadowData.normal, -shadowRayDir));
				#endif

					float32 G = cosThetaX * cosThetaY / squaredDist;

					directIllumination += lightSourceMat->Le * BRDF * G / pdfLight_area;
					if(directIllumination.x != directIllumination.x ||
					   directIllumination.y != directIllumination.y ||
					   directIllumination.z != directIllumination.z)
					{
						printf("NaN!\n");
					}
				}
			}
			directIllumination /= (float32)NUM_SHADOW_RAYS;

			// Because we are calculating for a non-emissive point, we can safely
			// add the direct illumination to this point.
			color += throughputTerm * directIllumination;
		}

		// Update the throughput term
		throughputTerm *= BRDF * cosTheta / pdfCosineWeightedHemisphere;
		// throughputTerm *= PI * BRDF;

		// Pick a new direction
		Vec2f randomVec2f = RandomVec2f();
		Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, data.normal);

		Vec3f point = data.point;
		ray = {point + EPSILON * data.normal, dir};
	}

	return color;
}