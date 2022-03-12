#pragma once
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
	uint32 numSpheres;

	Triangle *tris;
	uint32 numTris;

	uint32 *lightTris;
	uint32 numLightTris;

	BVH_Node *bvh;
};

Scene ConstructScene(Sphere *spheres, uint32 numSpheres, 
					 Triangle *modelTris, uint32 numTris,
					 uint32 *lightTris, uint32 numLightTris,
					 BVH_Node *bvh);

bool Intersect(Ray ray, Scene scene, HitData *data);