#pragma once

struct HitData
{
	float32 t;
	Vec3f normal;
	Vec3f point;
	struct Material *mat;
};

struct Sphere
{
	Vec3f origin;
	float32 radius;
	struct Material *mat;
};

bool SphereIntersect(Ray ray, Sphere sphere, HitData *data)
{
	Vec3f oc = ray.origin - sphere.origin;
	float32 a = Dot(ray.direction, ray.direction);
	float32 b = 2.0f * Dot(oc, ray.direction);
	float32 c = Dot(oc, oc) - sphere.radius * sphere.radius;
	float32 discriminant = b*b - 4.0f*a*c;
	if(discriminant >= 0)
	{
		float32 sqrtDiscriminant = sqrtf(discriminant);
		if(discriminant == 0) // 2 equal real solutions
		{
			float32 t = -b / (2.0f * a);
			if(t > TMIN && t < TMAX)
			{
				data->t = t;
				data->point = ray.origin + ray.direction * t;
				data->normal = data->point - sphere.origin;
				data->mat = sphere.mat;
				return true;
			}
		}
		else // 2 different real solutions
		{
			float32 t1 = (-b + sqrtDiscriminant) / (2.0f * a);
			float32 t2 = (-b - sqrtDiscriminant) / (2.0f * a);
			if(t1 > t2) 
			{
				float32 tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if(t1 > TMIN && t1 < TMAX)
			{
				data->t = t1;
				data->point = ray.origin + ray.direction * t1;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->mat = sphere.mat;
				return true;
			}
			else if(t2 > TMIN && t2 < TMAX)
			{
				data->t = t2;
				data->point = ray.origin + ray.direction * t2;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->mat = sphere.mat;
				return true;
			}
		}
	}

	return false;
}