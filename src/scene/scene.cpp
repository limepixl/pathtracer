#include "scene.hpp"
#include "sphere.hpp"
#include "material.hpp"

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return { obj, type };
}

Scene ConstructScene(Array<Sphere> spheres,
					 Array<Triangle> tris,
					 Array<uint32> light_tris,
					 Array<Material *> materials
					 /*Array<BVHNode> bvh_tree*/)
{
	return { spheres, tris, light_tris, materials/*, bvh_tree*/ };
}