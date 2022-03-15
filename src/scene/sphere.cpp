#include "sphere.hpp"
#include "../math/math.hpp"
#include "scene.hpp"
#include <math.h>

float Area(Sphere *sphere)
{
	return 4.0f * PI * sphere->radius * sphere->radius;
};

Sphere CreateSphere(Vec3f origin, float radius, Material *mat)
{
	Sphere result = {origin, radius, mat};
	return result;
}

bool SphereIntersect(Ray ray, Sphere sphere, HitData *data, float &tmax)
{
	Vec3f oc = ray.origin - sphere.origin;
	float a = Dot(ray.direction, ray.direction);
	float b = 2.0f * Dot(oc, ray.direction);
	float c = Dot(oc, oc) - sphere.radius * sphere.radius;
	float discriminant = b*b - 4.0f*a*c;
	if(discriminant >= 0)
	{
		float sqrtDiscriminant = sqrtf(discriminant);
		if(discriminant == 0) // 2 equal real solutions
		{
			float t = -b / (2.0f * a);
			if(t > TMIN && t < tmax)
			{
				tmax = t;
				data->t = t;
				data->point = ray.origin + ray.direction * t;
				data->normal = data->point - sphere.origin;
				data->mat = sphere.mat;
				return true;
			}
		}
		else // 2 different real solutions
		{
			float t1 = (-b + sqrtDiscriminant) / (2.0f * a);
			float t2 = (-b - sqrtDiscriminant) / (2.0f * a);
			if(t1 > t2) 
			{
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if(t1 > TMIN && t1 < tmax)
			{
				tmax = t1;
				data->t = t1;
				data->point = ray.origin + ray.direction * t1;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->mat = sphere.mat;
				return true;
			}
			else if(t2 > TMIN && t2 < tmax)
			{
				tmax = t2;
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
