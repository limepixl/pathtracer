#pragma once
#include "../defines.hpp"
#include "../math/math.hpp"
#include "ray.hpp"
#include "scene.hpp"

/*
	BVH implementation details
*/

struct AABB
{
	Vec3f bmin;
	Vec3f bmax;
};

bool operator==(const AABB &lhs, const AABB &rhs);
bool AABBIntersect(Ray ray, AABB aabb, float t);

struct BVH_Node
{
	BVH_Node *left, *right;
	uint32 numTris;
	uint32 index;
	AABB nodeAABB;
};

AABB ConstructAABBFromTris(struct Triangle *tris, uint32 numTris);
bool ConstructBVH(struct Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index = 0);
void IntersectBVHRecursive(Ray ray, Scene scene, BVH_Node *node, HitData *data, float &tmax, bool &hitAnything);
bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax);