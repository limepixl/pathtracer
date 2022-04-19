#include "scene.hpp"
#include "bvh.hpp"
#include "sphere.hpp"

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return { obj, type };
}

Scene ConstructScene(Array<Sphere> &spheres,
					 Array<Triangle> &tris,
					 Array<uint32> &lightTris,
					 BVH_Node *bvh)
{
	return { spheres, tris, lightTris, bvh };
}

bool Intersect(Ray ray, Scene scene, HitData *data)
{
	bool hitAnything = false;
	HitData resultData {};
	resultData.t = TMAX;

	float tmax = TMAX;

	for (uint32 i = 0; i < scene.spheres.size; i++)
	{
		HitData currentData = {};
		Sphere current = scene.spheres[i];
		bool intersect = SphereIntersect(ray, current, &currentData, tmax);
		if (intersect)
		{
			// Found closer hit, store it.
			hitAnything = true;
			resultData = currentData;

			resultData.objectIndex = i;
			resultData.objectType = ObjectType::SPHERE;
		}
	}

	hitAnything = hitAnything || IntersectBVHStack(ray, scene, &resultData, tmax);
	if (hitAnything)
	{
		*data = resultData;
	}

	return hitAnything;
}