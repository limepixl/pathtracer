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
	AABB nodeAABB; // 6 * 4 = 24 bytes
	uint32 index; // 4 bytes
	uint32 numTris; // 4 bytes
	BVH_Node *left, *right; // 16 bytes
	// = 48 bytes
};

AABB ConstructAABBFromTris(struct Triangle *tris, uint32 numTris);
bool ConstructBVHSweepSAH(struct Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index = 0);
bool ConstructBVHObjectMedian(struct Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index = 0);
void IntersectBVHRecursive(Ray ray, Scene scene, BVH_Node *node, HitData *data, float &tmax, bool &hitAnything);
bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax);
void DeleteBVH(BVH_Node *node);