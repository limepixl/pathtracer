#pragma once
#include "../core/array.hpp"
#include "../defines.hpp"
#include "../math/vec.hpp"
#include "ray.hpp"

enum ObjectType
{
	SPHERE = 0,
	TRIANGLE
};

struct HitData
{
	float t;
	Vec3f normal;
	Vec3f point;

	uint32 mat_index;

	uint32 object_index;
	ObjectType object_type;
};

enum class LightSourceType
{
	SPHERE,
	TRIANGLE
};

struct LightSource
{
	void *obj;
	LightSourceType type;
};

LightSource CreateLightSource(void *obj, LightSourceType type);

struct Sphere;
struct Triangle;
struct BVHNode;

struct Scene
{
	Array<Sphere> spheres;
	Array<Triangle> tris;
	Array<uint32> light_tris;
	Array<struct Material *> materials;
	// Array<BVHNode> bvh_tree;
};

Scene ConstructScene(Array<Sphere> spheres,
					 Array<Triangle> modelTris,
					 Array<uint32> light_tris,
					 Array<struct Material *> materials
					 /*Array<BVHNode> bvh_tree*/);

bool Intersect(Ray ray, Scene scene, HitData *data);