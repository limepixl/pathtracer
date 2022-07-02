#include "scene.hpp"
#include "bvh.hpp"
#include "sphere.hpp"

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return { obj, type };
}

Scene ConstructScene(Array<Sphere> spheres,
					 Array<Triangle> tris,
					 Array<uint32> light_tris
					 /*Array<BVHNode> bvh_tree*/)
{
	return { spheres, tris, light_tris/*, bvh_tree*/ };
}

bool Intersect(Ray ray, Scene scene, HitData *data)
{
	bool hit_anything = false;
	HitData result_data {};
	result_data.t = TMAX;

	float tmax = TMAX;

	for (uint32 i = 0; i < scene.spheres.size; i++)
	{
		HitData current_data = {};
		Sphere current = scene.spheres[i];
		bool intersect = SphereIntersect(ray, current, &current_data, tmax);
		if (intersect)
		{
			// Found closer hit, store it.
			hit_anything = true;
			result_data = current_data;

			result_data.object_index = i;
			result_data.object_type = ObjectType::SPHERE;
		}
	}

	// hit_anything = hit_anything || IntersectBVHStack(ray, scene, &result_data, tmax);
	if (hit_anything)
	{
		*data = result_data;
	}

	return hit_anything;
}