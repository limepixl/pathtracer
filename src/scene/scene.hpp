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

	struct Material *mat;

	uint32 objectIndex;
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
	Array<Sphere> spheres;
	Array<Triangle> tris;
	Array<uint32> lightTris;

	BVH_Node *bvh;
};

Scene ConstructScene(Array<Sphere> &spheres,
					 Array<Triangle> &modelTris,
					 Array<uint32> &lightTris,
					 BVH_Node *bvh);

bool Intersect(Ray ray, Scene scene, HitData *data);