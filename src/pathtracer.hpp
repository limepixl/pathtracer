#pragma once
#include "math/math.hpp"
#include "scene/material.hpp"
#include "scene/triangle.hpp"
#include "scene/sphere.hpp"

// A simple lerp between 2 colors
Vec3f SkyColor(Vec3f dir)
{
	Vec3f normalizedDir = NormalizeVec3f(dir);
    float t = 0.5f*(normalizedDir.y + 1.0f);

	Vec3f startColor = {1.0f, 1.0f, 1.0f};
	Vec3f endColor = {0.546f, 0.824f, 0.925f};
    return (1.0f-t) * startColor + t * endColor;
}

// Cast ray into the scene, have the ray bounce around
// a predetermined number of times accumulating radiance
// along the way, and return the color for the given pixel
Vec3f EstimatorPathTracingLambertian(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color {0.0f, 0.0f, 0.0f};

	// ( BRDF * dot(Nx, psi) ) / PDF(psi)
	Vec3f throughputTerm {1.0f, 1.0f, 1.0f};

	// Vignette effect (basically undoing We=1/cos^3(theta) )
	// float32 theta = acosf(-ray.direction.z);
	// throughputTerm = throughputTerm * powf(cosf(theta), 3);

	int8 numBounces = BOUNCE_COUNT;
	for(uint8 b = 0; b < numBounces; b++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if(!intersect) // ray goes off into infinity
		{
			if(b <= BOUNCE_COUNT)
				color += throughputTerm * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Material *mat = data.mat;

		// add the light that the material emits
		if(b <= BOUNCE_COUNT)
			color += throughputTerm * mat->Le;

		if(mat->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			Vec3f BRDF = PI * mat->diffuse;

			// update throughput
			// The PI is here because we are sampling w.r.t the pdf
			// p(psi) = cos(theta) / PI       (cosine weighted sampling)
			// This cosine term cancels out with the dot product in
			// the throughput term and all that is left is the BRDF
			throughputTerm *= BRDF;
			
			// pdf(psi) = cos(theta) / PI
			Vec2f randomVec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, data.normal);
	
			// Intersection point and new ray
			Vec3f point = data.point;
			ray = {point + EPSILON * data.normal, dir};
		}
		else if(mat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			Vec3f reflectedDir = Reflect(-ray.direction, data.normal);

			// Intersection point and new ray
			Vec3f point = data.point;
			ray = {point + EPSILON * data.normal, reflectedDir};

			// If we sample uniformly or with cosine weighted sampling, then
			// we would need to calculate the BRDF every single direction in the
			// hemisphere. If the direction is the reflected direction, then
			// the BRDF is 1, otherwise it is 0. But that is wasteful. Instead,
			// we *only* pick the reflected direction (not the case for glossy or
			// otherwise un-ideal reflective materials) so the pdf is 1, and the 
			// BRDF is 1. Because of this, we have an instant solution without
			// having to sample the hemisphere at all. The throughput term is then
			// BRDF*(Nx dot wi) = (Nx dot wi) = cosThetaX
			throughputTerm *= mat->specular;
		}
		else if(mat->type == MaterialType::MATERIAL_PHONG)
		{
			// TODO: fix the diffuse part
			float u = RandomNumberNormalizedPCG(rngptr);
			Vec3f uvec = CreateVec3f(u);
			if(uvec <= mat->diffuse * PI)
			{
				throughputTerm *= PI * mat->diffuse;

				// pdf(psi) = cos(theta) / PI
				Vec2f randomVec2f = RandomVec2fPCG(rngptr);
				Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, data.normal);

				ray = {data.point + EPSILON * data.normal, dir};
			}
			else if(uvec <= mat->diffuse * PI + mat->specular)
			{
				Vec3f Nx = data.normal;
				Vec3f x = data.point + EPSILON * data.normal;

				Vec3f reflectedDir = NormalizeVec3f(Reflect(-ray.direction, Nx));
				Mat3f tnb = ConstructTNB(reflectedDir);

				float inv = 1.0f / (mat->n_spec+1.0f);

				Vec2f randomVec2f = RandomVec2fPCG(rngptr);

				float cosAlpha = powf(randomVec2f.x, inv);
				float sinAlpha = sqrtf(1.0f - cosAlpha * cosAlpha);
				float phi = 2.0f * PI * randomVec2f.y;

				Vec3f dir = {sinAlpha * cosf(phi), cosAlpha, sinAlpha * sinf(phi)};

				dir = tnb * dir;
				ray = {x, dir};

				float cosThetaX = Max(0.0f, Dot(dir, Nx));

				float pdfTheta = ((mat->n_spec + 1.0f) / (2.0f * PI)) * powf(cosAlpha, mat->n_spec);
				float thetaTerm = powf(cosAlpha, mat->n_spec) * ((mat->n_spec + 2.0f) / (2.0f * PI)) / pdfTheta;
				
				throughputTerm *= mat->specular * cosThetaX * thetaTerm;
			}
		}
	}

	return color;
}

Vec3f EstimatorPathTracingLambertianNEE(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color = CreateVec3f(0.0f);
	Vec3f throughputTerm = CreateVec3f(1.0f);

	Material *oldMat = NULL;

	for(int16 bounce = 0; bounce < BOUNCE_COUNT; bounce++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if(!intersect)
		{
			// No intersection with scene, add env map contribution
			color += throughputTerm * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Vec3f &BRDF = data.mat->diffuse; // convenience
		Material *mat = data.mat;

		// (x->y dot Nx)
		float cosTheta = Dot(data.normal, ray.direction);
		float pdfCosineWeightedHemisphere = cosTheta / PI;

		// add light that is emitted from surface (but stop right afterwards)
		if(bounce <= BOUNCE_COUNT && mat->Le.x >= 0.5f)
		{
			// TODO: only add if the ray was added by a single lonely bounce from
			// the mirror instead of just the last bounce being the mirror?
			bool lastBounceWasReflected = (oldMat != NULL && oldMat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE);
			if(bounce == 0 || lastBounceWasReflected)
				color = mat->Le;

			return color;
		}
		// If there is at least 1 light source in the scene and the material
		// of the surface we hit is diffuse, we can use NEE.
		else if(scene.numLightTris > 0 && 
			(mat->type == MaterialType::MATERIAL_LAMBERTIAN || (mat->type == MaterialType::MATERIAL_PHONG && mat->specular == CreateVec3f(0.0f))))
		{
			// sample light sources for direct illumination
			Vec3f directIllumination = CreateVec3f(0.0f);
			for(int8 shadowRayIndex = 0; shadowRayIndex < NUM_SHADOW_RAYS; shadowRayIndex++)
			{
				// pick a light source
				float pdfPickLight = 1.0f / scene.numLightTris;

				int32 pickedLightSource = (int32)(pcg32_random_r(rngptr) % scene.numLightTris);
				// TODO: FIX THIS
				Triangle lightSource = scene.tris[scene.lightTris[pickedLightSource]];
				
				Material *lightSourceMat = lightSource.mat;
				float lightArea = Area(&lightSource);
				float pdfPickPointOnLight = 1.0f / lightArea;
				Vec3f y = MapToTriangle(RandomVec2fPCG(rngptr), lightSource);

				// PDF in terms of area, for picking point on light source k
				float pdfLight_area = pdfPickLight * pdfPickPointOnLight;

				// Just for clarity
				Vec3f x = data.point + EPSILON * data.normal;

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
					// printf("ERROR: Shadow ray didn't hit anything!\n");
					// break;

					// return CreateVec3f(5.0f, 0.0f, 5.0f);
				}

				// Visibility check means we have a clear line of sight!
				if(hitAnything && y == shadowData.point)
				{
					float squaredDist = Dot(distVec, distVec);

					// We want to only add light contribution from lights within 
					// the hemisphere solid angle above X, and not from lights behind it.
					float cosThetaX = Max(0.0f, Dot(data.normal, shadowRayDir));

					// We can sample the light from both sides, it doesn't have to
					// be a one-sided light source.
				#if TWO_SIDED_LIGHT
					float32 cosThetaY = Abs(Dot(shadowData.normal, shadowRayDir));
				#else
					float cosThetaY = Max(0.0f, Dot(shadowData.normal, -shadowRayDir));
				#endif

					float G = cosThetaX * cosThetaY / squaredDist;

					directIllumination += lightSourceMat->Le * BRDF * G / pdfLight_area;
					if(directIllumination.x != directIllumination.x ||
					   directIllumination.y != directIllumination.y ||
					   directIllumination.z != directIllumination.z)
					{
						// printf("NaN!\n");
					}
				}
			}
			directIllumination /= (float)NUM_SHADOW_RAYS;

			// Because we are calculating for a non-emissive point, we can safely
			// add the direct illumination to this point.
			color += throughputTerm * directIllumination;

			// Update the throughput term
			throughputTerm *= BRDF * cosTheta / pdfCosineWeightedHemisphere;
			// throughputTerm *= PI * BRDF;

			// Pick a new direction
			Vec2f randomVec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, data.normal);

			Vec3f point = data.point;
			ray = {point + EPSILON * data.normal, dir};
		}
		else if(mat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			Vec3f reflectedDir = Reflect(-ray.direction, data.normal);

			// Intersection point and new ray
			Vec3f point = data.point;
			ray = {point + EPSILON * data.normal, reflectedDir};

			// If we sample uniformly or with cosine weighted sampling, then
			// we would need to calculate the BRDF every single direction in the
			// hemisphere. If the direction is the reflected direction, then
			// the BRDF is 1, otherwise it is 0. But that is wasteful. Instead,
			// we *only* pick the reflected direction (not the case for glossy or
			// otherwise un-ideal reflective materials) so the pdf is 1, and the 
			// BRDF is 1. Because of this, we have an instant solution without
			// having to sample the hemisphere at all. The throughput term is then
			// BRDF*(Nx dot wi) = (Nx dot wi) = cosThetaX
			// throughputTerm *= Abs(cosTheta);
		}
		else if(mat->type == MaterialType::MATERIAL_PHONG)
		{
			// TODO: support the diffuse part

			Vec3f Nx = data.normal;
			Vec3f x = data.point + EPSILON * data.normal;

			Vec3f reflectedDir = NormalizeVec3f(Reflect(-ray.direction, Nx));
			Mat3f tnb = ConstructTNB(reflectedDir);

			float inv = 1.0f / (mat->n_spec+1.0f);

			Vec2f randomVec2f = RandomVec2fPCG(rngptr);

			float cosAlpha = powf(randomVec2f.x, inv);
			float sinAlpha = sqrtf(1.0f - cosAlpha * cosAlpha);
			float phi = 2.0f * PI * randomVec2f.y;

			Vec3f dir = {sinAlpha * cosf(phi), cosAlpha, sinAlpha * sinf(phi)};

			dir = tnb * dir;
			ray = {x, dir};

			float cosThetaX = Max(0.0f, Dot(dir, Nx));

			float pdfTheta = ((mat->n_spec + 1.0f) / (2.0f * PI)) * powf(cosAlpha, mat->n_spec);
			float thetaTerm = powf(cosAlpha, mat->n_spec) * ((mat->n_spec + 2.0f) / (2.0f * PI)) / pdfTheta;
			throughputTerm *= mat->specular * cosThetaX * thetaTerm;
		}

		oldMat = mat;
	}

	return color;
}

Vec3f EstimatorPathTracingMIS(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color = CreateVec3f(0.0f);
	Vec3f throughputTerm = CreateVec3f(1.0f);

	HitData data = {};
	bool intersect = Intersect(ray, scene, &data);
	if(!intersect)
	{
		// No intersection with scene, add env map contribution
		color += throughputTerm * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
		return color;
	}

	Vec3f y = ray.origin + ray.direction * data.t + EPSILON * data.normal;
	Vec3f normalY = data.normal;
	Material *matY = data.mat;

	// Add light contribution from first bounce if it hit a light source
	color += throughputTerm * matY->Le;

	int8 numBounces = BOUNCE_COUNT + 1;
	for(uint8 b = 1; b < numBounces; b++)
	{
		Vec3f x = y;
		Vec3f normalX = normalY;
		Material *matX = matY;

		// If there is at least 1 light source in the scene, and the material of the
		// surface is diffuse, we can calculate the direct light contribution (NEE)
		bool canUseNEE = scene.numLightTris > 0 && 
		                 (matX->type == MaterialType::MATERIAL_LAMBERTIAN || 
						 (matX->type == MaterialType::MATERIAL_PHONG && matX->specular == CreateVec3f(0.0f))) && 
						 matX->Le.x < 0.1f;
		
		if(canUseNEE)
		{
			// sample light sources for direct illumination
			for(int8 shadowRayIndex = 0; shadowRayIndex < NUM_SHADOW_RAYS; shadowRayIndex++)
			{
				// pick a light source
				float pdfPickLight = 1.0f / scene.numLightTris;

				uint32 r = pcg32_random_r(rngptr);
				int32 pickedLightSource = (int32)(r % scene.numLightTris);
				Triangle lightSource = scene.tris[scene.lightTris[pickedLightSource]];
				
				Material *lightSourceMat = lightSource.mat;
				Vec3f y_nee = MapToTriangle(RandomVec2fPCG(rngptr), lightSource);
				float lightArea = Area(&lightSource);

				// Just for clarity
				Vec3f x_nee = x;

				// Send out a shadow ray in direction x->y
				Vec3f distVec = y_nee - x_nee;
				Vec3f shadowRayDir = NormalizeVec3f(distVec);
				Ray shadowRay = {x_nee, shadowRayDir};

				float squaredDist = Dot(distVec, distVec);

				// We want to only add light contribution from lights within 
				// the hemisphere solid angle above X, and not from lights behind it.
				float cosThetaX = Max(0.0f, Dot(normalX, shadowRayDir));

				// Check if ray hits anything before hitting the light source
				HitData shadowData = {};
				bool hitAnything = Intersect(shadowRay, scene, &shadowData);
				if(!hitAnything)
				{
					// return CreateVec3f(5.0f, 0.0f, 5.0f);
				}

				// Visibility check means we have a clear line of sight!
				if(hitAnything && y_nee == shadowData.point)
				{
				#if TWO_SIDED_LIGHT
					float32 cosThetaY = Dot(shadowData.normal, -shadowRay.direction);
					Vec3f normalY_nee = shadowData.normal * Sign(cosThetaY);
					cosThetaY = Abs(cosThetaY);
				#else
					float cosThetaY = Max(0.0f, Dot(shadowData.normal, -shadowRay.direction));
					Vec3f normalY_nee = shadowData.normal;
					if(cosThetaY > 0.0f)
				#endif
					{
						float pdfPickPointOnLight = 1.0f / lightArea;

						// PDF in terms of area, for picking point on light source k
						float pdfLight_area = pdfPickLight * pdfPickPointOnLight;

						// PDF in terms of solid angle, for picking point on light source k
						float pdfLight_sa = pdfLight_area * squaredDist / cosThetaY;

						// PDF in terms of solid angle, for picking ray from cos. weighted hemisphere
						float pdfBSDFsolidAngle = cosThetaX / PI;

						// weight for NEE 
						// float32 wNEE = BalanceHeuristic(pdfLight_sa, pdfBSDFsolidAngle) / pdfLight_sa;
						float wNEE = 1.0f / (pdfLight_sa + pdfBSDFsolidAngle);

						color += lightSourceMat->Le * matX->diffuse * cosThetaX * throughputTerm * wNEE;
					}
				}
			}
		}

		if(matX->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			// Pick a new direction
			Vec2f randomVec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(randomVec2f, normalX);
			ray = {x + EPSILON * normalX, dir};
		}
		else if(matX->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			// Pick the reflected direciton
			Vec3f reflectedDir = Reflect(-ray.direction, normalX);
			ray = {x + EPSILON * normalX, reflectedDir};
		}
		else if(matX->type == MaterialType::MATERIAL_PHONG)
		{
			// Pick direction in the cosine lobe around reflected dir
			Vec3f reflectedDir = Reflect(-ray.direction, normalX);
			Mat3f tnb = ConstructTNB(reflectedDir);

			float inv = 1.0f / (matX->n_spec+1.0f);

			Vec2f randomVec2f = RandomVec2fPCG(rngptr);
			float cosAlpha = powf(randomVec2f.x, inv);
			float sinAlpha = sqrtf(1.0f - cosAlpha * cosAlpha);
			float phi = 2.0f * PI * randomVec2f.y;

			Vec3f dir = {sinAlpha * cosf(phi), cosAlpha, sinAlpha * sinf(phi)};
			dir = tnb * dir;
			ray = {x, dir};
		}

		float cosThetaX = Max(0.0f, Dot(normalX, ray.direction));

		intersect = Intersect(ray, scene, &data);
		if(!intersect)
		{
			// No intersection with scene, add env map contribution
			if(b <= BOUNCE_COUNT)
			{
				if(matX->type == MATERIAL_LAMBERTIAN)
					color += throughputTerm * PI * matX->diffuse * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
				else if(matX->type == MATERIAL_IDEAL_REFLECTIVE)
					color += throughputTerm * matX->specular * SkyColor(ray.direction) * cosThetaX * ENVIRONMENT_MAP_LE;
				else if(matX->type == MATERIAL_PHONG)
				{
					float thetaTerm = 1.0f + 1.0f / (matX->n_spec + 1.0f);
					color += throughputTerm * matX->specular * SkyColor(ray.direction) * cosThetaX * thetaTerm * ENVIRONMENT_MAP_LE;
				}
				
				break;
			}
		}

	#if TWO_SIDED_LIGHT
		float32 cosThetaY = Dot(data.normal, -ray.direction);
		normalY = data.normal * Sign(cosThetaY);
		cosThetaY = Abs(cosThetaY);
	#else
        float cosThetaY = Max(0.0f, -Dot(data.normal, ray.direction));
		normalY = data.normal;
	#endif

		float pdfBSDFsolidAngle = 0.0f;
		if(matX->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			// Because the new ray direction is sampled using cosine weighted hemisphere
			// sampling, the pdf is cosTheta / PI
			pdfBSDFsolidAngle = cosThetaY / PI;
		}
		else if(matX->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			// We only sample the single direction, so the pdf is 1 for that
			// reflected direction
			pdfBSDFsolidAngle = 1.0f;
		}
		else if(matX->type == MaterialType::MATERIAL_PHONG)
		{
			// NOTE: it's this value because I assume the terms present in the pdf
			// cancel out with the terms present in the specular term of the BRDF
			// because I importance sample the specular term 
			pdfBSDFsolidAngle = 1.0f;
		}

		y = ray.origin + ray.direction * data.t + EPSILON * normalY;
		matY = data.mat;

		float wBSDF = 1.0f;

		// If we can use NEE on the hit surface
		if(matX->type == MaterialType::MATERIAL_LAMBERTIAN && matX->Le.x < 0.1f && cosThetaY > 0.0f)
		{
			// If the hit surface is a light source, we need to calculate
			// the pdf for NEE
			if(matY->Le.x > 0.1f && scene.numLightTris > 0)
			{
				float pdfNEE_area = 0.0f;
				switch(data.objectType)
				{
				case ObjectType::SPHERE:
					pdfNEE_area = 1.0f / Area(&scene.spheres[data.objectIndex]);
					break;
				case ObjectType::TRIANGLE:
					pdfNEE_area = 1.0f / Area(&scene.tris[data.objectIndex]);
					break;
				default:
					// printf("Light source type unsupported for sampling!\n");
					break;
				};

				pdfNEE_area /= (float)(scene.numLightTris);
				float pdfNEE_solidAngle = pdfNEE_area * data.t * data.t / cosThetaY;
				wBSDF = BalanceHeuristic(pdfBSDFsolidAngle, pdfNEE_solidAngle);
			}

			if(b <= BOUNCE_COUNT)
				color += throughputTerm * matX->diffuse * matY->Le * PI * wBSDF;
		}
		else if(b <= BOUNCE_COUNT && cosThetaY > 0.0f)
		{
			if(matX->type == MATERIAL_PHONG)
			{
				// Sampled w.r.t the cosine term 
				Vec3f diffusePart = PI * matX->diffuse;

				// Sampled w.r.t the BRDF
				float thetaTerm = 1.0f + 1.0f / (matX->n_spec + 1.0f);
				Vec3f specularPart = matX->specular * thetaTerm * cosThetaX;

				color += throughputTerm * (diffusePart + specularPart) * matY->Le * cosThetaY;
			}
		}

		if(matX->type == MaterialType::MATERIAL_LAMBERTIAN)
			throughputTerm *= PI * matX->diffuse;
		else if(matX->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
			throughputTerm *= matX->specular;
		else if(matX->type == MaterialType::MATERIAL_PHONG)
		{
			float thetaTerm = 1.0f + 1.0f / (matX->n_spec + 1.0f);
			throughputTerm *= matX->specular * cosThetaX * thetaTerm;
		}
	}

	return color;
}
