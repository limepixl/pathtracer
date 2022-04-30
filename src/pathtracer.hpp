#pragma once
#include "math/math.hpp"
#include "scene/material.hpp"
#include "scene/scene.hpp"
#include "scene/sphere.hpp"
#include "scene/triangle.hpp"
#include <cmath>

// A simple lerp between 2 colors
Vec3f SkyColor(Vec3f dir)
{
	Vec3f normalized_dir = NormalizeVec3f(dir);
	float t = 0.5f * (normalized_dir.y + 1.0f);

	Vec3f start_color = { 1.0f, 1.0f, 1.0f };
	Vec3f end_color = { 0.546f, 0.824f, 0.925f };
	return (1.0f - t) * start_color + t * end_color;
}

// Cast ray into the scene, have the ray bounce around
// a predetermined number of times accumulating radiance
// along the way, and return the color for the given pixel
Vec3f EstimatorPathTracingLambertian(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color { 0.0f, 0.0f, 0.0f };

	// ( BRDF * dot(Nx, psi) ) / PDF(psi)
	Vec3f throughput_term { 1.0f, 1.0f, 1.0f };

	// Vignette effect (basically undoing We=1/cos^3(theta) )
	// float32 theta = acosf(-ray.direction.z);
	// throughput_term = throughput_term * powf(cosf(theta), 3);

	int8 num_bounces = BOUNCE_COUNT;
	for (uint8 b = 0; b < num_bounces; b++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if (!intersect) // ray goes off into infinity
		{
			if (b <= BOUNCE_COUNT)
				color += throughput_term * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Material *mat = data.mat;

		// add the light that the material emits
		if (b <= BOUNCE_COUNT)
			color += throughput_term * mat->Le;

		if (mat->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			Vec3f BRDF = PI * mat->diffuse;

			// update throughput
			// The PI is here because we are sampling w.r.t the pdf
			// p(psi) = cos(theta) / PI       (cosine weighted sampling)
			// This cosine term cancels out with the dot product in
			// the throughput term and all that is left is the BRDF
			throughput_term *= BRDF;

			// pdf(psi) = cos(theta) / PI
			Vec2f random_vec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(random_vec2f, data.normal);

			// Intersection point and new ray
			Vec3f point = data.point;
			ray = { point + EPSILON * data.normal, dir, 1.0f / dir };
		}
		else if (mat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			Vec3f reflected_dir = Reflect(-ray.direction, data.normal);

			// Intersection point and new ray
			Vec3f point = data.point;
			ray = { point + EPSILON * data.normal, reflected_dir, 1.0f / reflected_dir };

			// If we sample uniformly or with cosine weighted sampling, then
			// we would need to calculate the BRDF every single direction in the
			// hemisphere. If the direction is the reflected direction, then
			// the BRDF is 1, otherwise it is 0. But that is wasteful. Instead,
			// we *only* pick the reflected direction (not the case for glossy or
			// otherwise un-ideal reflective materials) so the pdf is 1, and the
			// BRDF is 1. Because of this, we have an instant solution without
			// having to sample the hemisphere at all. The throughput term is then
			// BRDF*(Nx dot wi) = (Nx dot wi) = cos_theta_x
			throughput_term *= mat->specular;
		}
		else if (mat->type == MaterialType::MATERIAL_PHONG)
		{
			// TODO: fix the diffuse part
			float u = RandomNumberNormalizedPCG(rngptr);
			Vec3f uvec = CreateVec3f(u);
			if (uvec <= mat->diffuse * PI)
			{
				throughput_term *= PI * mat->diffuse;

				// pdf(psi) = cos(theta) / PI
				Vec2f random_vec2f = RandomVec2fPCG(rngptr);
				Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(random_vec2f, data.normal);

				ray = { data.point + EPSILON * data.normal, dir, 1.0f / dir };
			}
			else if (uvec <= mat->diffuse * PI + mat->specular)
			{
				Vec3f Nx = data.normal;
				Vec3f x = data.point + EPSILON * data.normal;

				Vec3f reflected_dir = NormalizeVec3f(Reflect(-ray.direction, Nx));
				Mat3f tnb = ConstructTNB(reflected_dir);

				float inv = 1.0f / (mat->n_spec + 1.0f);

				Vec2f random_vec2f = RandomVec2fPCG(rngptr);

				float cos_alpha = powf(random_vec2f.x, inv);
				float sin_alpha = sqrtf(1.0f - cos_alpha * cos_alpha);
				float phi = 2.0f * PI * random_vec2f.y;

				Vec3f dir = { sin_alpha * cosf(phi), cos_alpha, sin_alpha * sinf(phi) };

				dir = tnb * dir;
				ray = { x, dir, 1.0f / dir };

				float cos_theta_x = Max(0.0f, Dot(dir, Nx));

				float pdf_theta = ((mat->n_spec + 1.0f) / (2.0f * PI)) * powf(cos_alpha, mat->n_spec);
				float theta_term = powf(cos_alpha, mat->n_spec) * ((mat->n_spec + 2.0f) / (2.0f * PI)) / pdf_theta;

				throughput_term *= mat->specular * cos_theta_x * theta_term;
			}
		}
	}

	return color;
}

Vec3f EstimatorPathTracingLambertianNEE(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color = CreateVec3f(0.0f);
	Vec3f throughput_term = CreateVec3f(1.0f);

	Material *old_mat = nullptr;

	for (int16 bounce = 0; bounce < BOUNCE_COUNT; bounce++)
	{
		HitData data = {};
		bool intersect = Intersect(ray, scene, &data);
		if (!intersect)
		{
			// No intersection with scene, add env map contribution
			color += throughput_term * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
			break;
		}

		Vec3f &BRDF = data.mat->diffuse; // convenience
		Material *mat = data.mat;

		// (x->y dot Nx)
		float cos_theta = Dot(data.normal, ray.direction);
		float pdf_cos_weighted_hemisphere = cos_theta / PI;

		// add light that is emitted from surface (but stop right afterwards)
		if (bounce <= BOUNCE_COUNT && mat->Le.x >= 0.5f)
		{
			// TODO: only add if the ray was added by a single lonely bounce from
			// the mirror instead of just the last bounce being the mirror?
			bool last_bounce_reflected = (old_mat != nullptr && old_mat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE);
			if (bounce == 0 || last_bounce_reflected)
				color = mat->Le;

			return color;
		}
		// If there is at least 1 light source in the scene and the material
		// of the surface we hit is diffuse, we can use NEE.
		else if (scene.light_tris.size > 0 && (mat->type == MaterialType::MATERIAL_LAMBERTIAN || (mat->type == MaterialType::MATERIAL_PHONG && mat->specular == CreateVec3f(0.0f))))
		{
			// sample light sources for direct illumination
			Vec3f direct_illumination = CreateVec3f(0.0f);
			for (int8 shadow_ray_index = 0; shadow_ray_index < NUM_SHADOW_RAYS; shadow_ray_index++)
			{
				// pick a light source
				float pdf_pick_light = 1.0f / (float)scene.light_tris.size;

				int32 picked_light_source = (int32)(pcg32_random_r(rngptr) % scene.light_tris.size);
				// TODO: FIX THIS
				Triangle light_source = scene.tris[scene.light_tris[picked_light_source]];

				Material *light_source_mat = light_source.mat;
				float light_area = Area(&light_source);
				float pdf_pick_point_on_light = 1.0f / light_area;
				Vec3f y = MapToTriangle(RandomVec2fPCG(rngptr), light_source);

				// PDF in terms of area, for picking point on light source k
				float pdf_light_area = pdf_pick_light * pdf_pick_point_on_light;

				// Just for clarity
				Vec3f x = data.point + EPSILON * data.normal;

				// Send out a shadow ray in direction x->y
				Vec3f dist_vec = y - x;
				Vec3f shadow_ray_dir = NormalizeVec3f(dist_vec);
				Ray shadow_ray = { x, shadow_ray_dir, 1.0f / shadow_ray_dir };

				// Check if ray hits anything before hitting the light source
				HitData shadow_data = {};
				bool hit_anything = Intersect(shadow_ray, scene, &shadow_data);
				if (!hit_anything)
				{
					// It shouldn't be possible to send a ray towards the light
					// source and not hit anything in the scene, even the light
					// printf("ERROR: Shadow ray didn't hit anything!\n");
					// break;

					// return CreateVec3f(5.0f, 0.0f, 5.0f);
				}

				// Visibility check means we have a clear line of sight!
				if (hit_anything && y == shadow_data.point)
				{
					float squared_dist = Dot(dist_vec, dist_vec);

					// We want to only add light contribution from lights within
					// the hemisphere solid angle above X, and not from lights behind it.
					float cos_theta_x = Max(0.0f, Dot(data.normal, shadow_ray_dir));

					// We can sample the light from both sides, it doesn't have to
					// be a one-sided light source.
#if TWO_SIDED_LIGHT
					float32 cos_theta_y = Abs(Dot(shadow_data.normal, shadowRayDir));
#else
					float cos_theta_y = Max(0.0f, Dot(shadow_data.normal, -shadow_ray_dir));
#endif

					float G = cos_theta_x * cos_theta_y / squared_dist;

					direct_illumination += light_source_mat->Le * BRDF * G / pdf_light_area;
					if (direct_illumination.x != direct_illumination.x || direct_illumination.y != direct_illumination.y || direct_illumination.z != direct_illumination.z)
					{
						// printf("NaN!\n");
					}
				}
			}
			direct_illumination /= (float)NUM_SHADOW_RAYS;

			// Because we are calculating for a non-emissive point, we can safely
			// add the direct illumination to this point.
			color += throughput_term * direct_illumination;

			// Update the throughput term
			throughput_term *= BRDF * cos_theta / pdf_cos_weighted_hemisphere;
			// throughput_term *= PI * BRDF;

			// Pick a new direction
			Vec2f random_vec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(random_vec2f, data.normal);

			Vec3f point = data.point;
			ray = { point + EPSILON * data.normal, dir, 1.0f / dir };
		}
		else if (mat->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			Vec3f reflected_dir = Reflect(-ray.direction, data.normal);

			// Intersection point and new ray
			Vec3f point = data.point;
			ray = { point + EPSILON * data.normal, reflected_dir, 1.0f / reflected_dir };

			// If we sample uniformly or with cosine weighted sampling, then
			// we would need to calculate the BRDF every single direction in the
			// hemisphere. If the direction is the reflected direction, then
			// the BRDF is 1, otherwise it is 0. But that is wasteful. Instead,
			// we *only* pick the reflected direction (not the case for glossy or
			// otherwise un-ideal reflective materials) so the pdf is 1, and the
			// BRDF is 1. Because of this, we have an instant solution without
			// having to sample the hemisphere at all. The throughput term is then
			// BRDF*(Nx dot wi) = (Nx dot wi) = cos_theta_x
			// throughput_term *= Abs(cos_theta);
		}
		else if (mat->type == MaterialType::MATERIAL_PHONG)
		{
			// TODO: support the diffuse part

			Vec3f Nx = data.normal;
			Vec3f x = data.point + EPSILON * data.normal;

			Vec3f reflected_dir = NormalizeVec3f(Reflect(-ray.direction, Nx));
			Mat3f tnb = ConstructTNB(reflected_dir);

			float inv = 1.0f / (mat->n_spec + 1.0f);

			Vec2f random_vec2f = RandomVec2fPCG(rngptr);

			float cos_alpha = powf(random_vec2f.x, inv);
			float sin_alpha = sqrtf(1.0f - cos_alpha * cos_alpha);
			float phi = 2.0f * PI * random_vec2f.y;

			Vec3f dir = { sin_alpha * cosf(phi), cos_alpha, sin_alpha * sinf(phi) };

			dir = tnb * dir;
			ray = { x, dir, 1.0f / dir };

			float cos_theta_x = Max(0.0f, Dot(dir, Nx));

			float pdf_theta = ((mat->n_spec + 1.0f) / (2.0f * PI)) * powf(cos_alpha, mat->n_spec);
			float theta_term = powf(cos_alpha, mat->n_spec) * ((mat->n_spec + 2.0f) / (2.0f * PI)) / pdf_theta;
			throughput_term *= mat->specular * cos_theta_x * theta_term;
		}

		old_mat = mat;
	}

	return color;
}

Vec3f EstimatorPathTracingMIS(Ray ray, Scene scene, pcg32_random_t *rngptr)
{
	Vec3f color = CreateVec3f(0.0f);
	Vec3f throughput_term = CreateVec3f(1.0f);

	HitData data = {};
	bool intersect = Intersect(ray, scene, &data);
	if (!intersect)
	{
		// No intersection with scene, add env map contribution
		color += throughput_term * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
		return color;
	}

	Vec3f y = ray.origin + ray.direction * data.t + EPSILON * data.normal;
	Vec3f normal_y = data.normal;
	Material *mat_y = data.mat;

	// Add light contribution from first bounce if it hit a light source
	color += throughput_term * mat_y->Le;

	int8 num_bounces = BOUNCE_COUNT + 1;
	for (uint8 b = 1; b < num_bounces; b++)
	{
		Vec3f x = y;
		Vec3f normal_x = normal_y;
		Material *mat_x = mat_y;

		// If there is at least 1 light source in the scene, and the material of the
		// surface is diffuse, we can calculate the direct light contribution (NEE)
		bool can_use_NEE = scene.light_tris.size > 0 && (mat_x->type == MaterialType::MATERIAL_LAMBERTIAN || (mat_x->type == MaterialType::MATERIAL_PHONG && mat_x->specular == CreateVec3f(0.0f))) && mat_x->Le.x < 0.1f;

		if (can_use_NEE)
		{
			// sample light sources for direct illumination
			for (int8 shadow_ray_index = 0; shadow_ray_index < NUM_SHADOW_RAYS; shadow_ray_index++)
			{
				// pick a light source
				float pdf_pick_light = 1.0f / (float)scene.light_tris.size;

				uint32 r = pcg32_random_r(rngptr);
				int32 picked_light_source = (int32)(r % scene.light_tris.size);
				Triangle light_source = scene.tris[scene.light_tris[picked_light_source]];

				Material *light_source_mat = light_source.mat;
				Vec3f y_nee = MapToTriangle(RandomVec2fPCG(rngptr), light_source);
				float light_area = Area(&light_source);

				// Just for clarity
				Vec3f x_nee = x;

				// Send out a shadow ray in direction x->y
				Vec3f dist_vec = y_nee - x_nee;
				Vec3f shadow_ray_dir = NormalizeVec3f(dist_vec);
				Ray shadow_ray = { x_nee, shadow_ray_dir, 1.0f / shadow_ray_dir };

				float squared_dist = Dot(dist_vec, dist_vec);

				// We want to only add light contribution from lights within
				// the hemisphere solid angle above X, and not from lights behind it.
				float cos_theta_x = Max(0.0f, Dot(normal_x, shadow_ray_dir));

				// Check if ray hits anything before hitting the light source
				HitData shadow_data = {};
				bool hit_anything = Intersect(shadow_ray, scene, &shadow_data);
				if (!hit_anything)
				{
					// return CreateVec3f(5.0f, 0.0f, 5.0f);
				}

				// Visibility check means we have a clear line of sight!
				if (hit_anything && y_nee == shadow_data.point)
				{
#if TWO_SIDED_LIGHT
					float32 cos_theta_y = Dot(shadow_data.normal, -shadow_ray.direction);
					Vec3f normalY_nee = shadow_data.normal * Sign(cos_theta_y);
					cos_theta_y = Abs(cos_theta_y);
#else
					float cos_theta_y = Max(0.0f, Dot(shadow_data.normal, -shadow_ray.direction));
					// Vec3f normalY_nee = shadow_data.normal;
					if (cos_theta_y > 0.0f)
#endif
					{
						float pdf_pick_point_on_light = 1.0f / light_area;

						// PDF in terms of area, for picking point on light source k
						float pdf_light_area = pdf_pick_light * pdf_pick_point_on_light;

						// PDF in terms of solid angle, for picking point on light source k
						float pdf_light_sa = pdf_light_area * squared_dist / cos_theta_y;

						// PDF in terms of solid angle, for picking ray from cos. weighted hemisphere
						float pdf_BSDF_sa = cos_theta_x / PI;

						// weight for NEE
						// float32 wNEE = BalanceHeuristic(pdf_light_sa, pdf_BSDF_sa) / pdf_light_sa;
						float wNEE = 1.0f / (pdf_light_sa + pdf_BSDF_sa);

						color += light_source_mat->Le * mat_x->diffuse * cos_theta_x * throughput_term * wNEE;
					}
				}
			}
		}

		if (mat_x->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			// Pick a new direction
			Vec2f random_vec2f = RandomVec2fPCG(rngptr);
			Vec3f dir = MapToUnitHemisphereCosineWeightedCriver(random_vec2f, normal_x);
			ray = { x + EPSILON * normal_x, dir, 1.0f / dir };
		}
		else if (mat_x->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			// Pick the reflected direciton
			Vec3f reflected_dir = Reflect(-ray.direction, normal_x);
			ray = { x + EPSILON * normal_x, reflected_dir, 1.0f / reflected_dir };
		}
		else if (mat_x->type == MaterialType::MATERIAL_PHONG)
		{
			// Pick direction in the cosine lobe around reflected dir
			Vec3f reflected_dir = Reflect(-ray.direction, normal_x);
			Mat3f tnb = ConstructTNB(reflected_dir);

			float inv = 1.0f / (mat_x->n_spec + 1.0f);

			Vec2f random_vec2f = RandomVec2fPCG(rngptr);
			float cos_alpha = powf(random_vec2f.x, inv);
			float sin_alpha = sqrtf(1.0f - cos_alpha * cos_alpha);
			float phi = 2.0f * PI * random_vec2f.y;

			Vec3f dir = { sin_alpha * cosf(phi), cos_alpha, sin_alpha * sinf(phi) };
			dir = tnb * dir;
			ray = { x, dir, 1.0f / dir };
		}

		float cos_theta_x = Max(0.0f, Dot(normal_x, ray.direction));

		intersect = Intersect(ray, scene, &data);
		if (!intersect)
		{
			// No intersection with scene, add env map contribution
			if (b <= BOUNCE_COUNT)
			{
				if (mat_x->type == MATERIAL_LAMBERTIAN)
					color += throughput_term * PI * mat_x->diffuse * SkyColor(ray.direction) * ENVIRONMENT_MAP_LE;
				else if (mat_x->type == MATERIAL_IDEAL_REFLECTIVE)
					color += throughput_term * mat_x->specular * SkyColor(ray.direction) * cos_theta_x * ENVIRONMENT_MAP_LE;
				else if (mat_x->type == MATERIAL_PHONG)
				{
					float theta_term = 1.0f + 1.0f / (mat_x->n_spec + 1.0f);
					color += throughput_term * mat_x->specular * SkyColor(ray.direction) * cos_theta_x * theta_term * ENVIRONMENT_MAP_LE;
				}

				break;
			}
		}

#if TWO_SIDED_LIGHT
		float32 cos_theta_y = Dot(data.normal, -ray.direction);
		normal_y = data.normal * Sign(cos_theta_y);
		cos_theta_y = Abs(cos_theta_y);
#else
		float cos_theta_y = Max(0.0f, -Dot(data.normal, ray.direction));
		normal_y = data.normal;
#endif

		float pdf_BSDF_sa = 0.0f;
		if (mat_x->type == MaterialType::MATERIAL_LAMBERTIAN)
		{
			// Because the new ray direction is sampled using cosine weighted hemisphere
			// sampling, the pdf is cos_theta / PI
			pdf_BSDF_sa = cos_theta_y / PI;
		}
		else if (mat_x->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
		{
			// We only sample the single direction, so the pdf is 1 for that
			// reflected direction
			pdf_BSDF_sa = 1.0f;
		}
		else if (mat_x->type == MaterialType::MATERIAL_PHONG)
		{
			// NOTE: it's this value because I assume the terms present in the pdf
			// cancel out with the terms present in the specular term of the BRDF
			// because I importance sample the specular term
			pdf_BSDF_sa = 1.0f;
		}

		y = ray.origin + ray.direction * data.t + EPSILON * normal_y;
		mat_y = data.mat;

		float wBSDF = 1.0f;

		// If we can use NEE on the hit surface
		if (mat_x->type == MaterialType::MATERIAL_LAMBERTIAN && mat_x->Le.x < 0.1f && cos_theta_y > 0.0f)
		{
			// If the hit surface is a light source, we need to calculate
			// the pdf for NEE
			if (mat_y->Le.x > 0.1f && scene.light_tris.size > 0)
			{
				float pdf_NEE_area = 0.0f;
				switch (data.object_type)
				{
				case ObjectType::SPHERE:
					pdf_NEE_area = 1.0f / Area(&scene.spheres[data.object_index]);
					break;
				case ObjectType::TRIANGLE:
					pdf_NEE_area = 1.0f / Area(&scene.tris[data.object_index]);
					break;
				default:
					// printf("Light source type unsupported for sampling!\n");
					break;
				};

				pdf_NEE_area /= (float)(scene.light_tris.size);
				float pdf_NEE_sa = pdf_NEE_area * data.t * data.t / cos_theta_y;
				wBSDF = BalanceHeuristic(pdf_BSDF_sa, pdf_NEE_sa);
			}

			if (b <= BOUNCE_COUNT)
				color += throughput_term * mat_x->diffuse * mat_y->Le * PI * wBSDF;
		}
		else if (b <= BOUNCE_COUNT && cos_theta_y > 0.0f)
		{
			if (mat_x->type == MATERIAL_PHONG)
			{
				// Sampled w.r.t the cosine term
				Vec3f diffuse_part = PI * mat_x->diffuse;

				// Sampled w.r.t the BRDF
				float theta_term = 1.0f + 1.0f / (mat_x->n_spec + 1.0f);
				Vec3f specular_part = mat_x->specular * theta_term * cos_theta_x;

				color += throughput_term * (diffuse_part + specular_part) * mat_y->Le * cos_theta_y;
			}
		}

		if (mat_x->type == MaterialType::MATERIAL_LAMBERTIAN)
			throughput_term *= PI * mat_x->diffuse;
		else if (mat_x->type == MaterialType::MATERIAL_IDEAL_REFLECTIVE)
			throughput_term *= mat_x->specular;
		else if (mat_x->type == MaterialType::MATERIAL_PHONG)
		{
			float theta_term = 1.0f + 1.0f / (mat_x->n_spec + 1.0f);
			throughput_term *= mat_x->specular * cos_theta_x * theta_term;
		}
	}

	return color;
}
