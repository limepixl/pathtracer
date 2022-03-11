#pragma once
#include "defines.hpp"
#include "vec.hpp"
#include "ray.hpp"

enum ObjectType
{
	SPHERE = 0,
	TRIANGLE
};

struct HitData
{
	float32 t;
	Vec3f normal;
	Vec3f point;

	struct Material *mat;

	int32 objectIndex;
	ObjectType objectType;
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
struct BVH_Node;

struct Scene
{
	Sphere *spheres;
	int32 numSpheres;

	Triangle *modelTris;
	int32 numTris;

	uint32 *lightTris;
	int32 numLightTris;

	BVH_Node *bvh;
};

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Triangle *modelTris, int32 numTris,
					 uint32 *lightTris, int32 numLightTris,
					 BVH_Node *bvh);

bool Intersect(Ray ray, Scene scene, HitData *data);