#include "scene.hpp"
#include "bvh.hpp"

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return {obj, type};
}

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Triangle *tris, int32 numTris,
					 uint32 *lightTris, int32 numLightTris,
					 BVH_Node *bvh)
{
	return {spheres, numSpheres, 
			tris, numTris,
			lightTris, numLightTris, bvh};
}

bool Intersect(Ray ray, Scene scene, HitData *data)
{
	bool hitAnything = false;
	HitData resultData = {TMAX};

	float32 tmax = TMAX;

	/*
	for(int32 i = 0; i < scene.numSpheres; i++)
	{
		HitData currentData = {};
		Sphere current = scene.spheres[i];
		bool intersect = SphereIntersect(ray, current, &currentData, tmax);
		if(intersect)
		{
			// Found closer hit, store it.
			hitAnything = true;
			resultData = currentData;
			
			resultData.objectIndex = i;
			resultData.objectType = ObjectType::SPHERE;
		}
	}
	*/

	IntersectBVH(ray, scene, scene.bvh, &resultData, tmax, hitAnything);

	*data = resultData;
	return hitAnything;
}