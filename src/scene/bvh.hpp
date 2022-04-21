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
	uint32 first_tri; // 4 bytes
	uint32 numTris; // 4 bytes
	int32 left, right; // 8 bytes
	// = 40 bytes
};

AABB ConstructAABBFromTris(struct Triangle *tris, uint32 numTris);
bool ConstructBVHSweepSAH(Triangle *tris, uint32 numTris, Array<BVH_Node> &bvh_tree, uint32 index);
bool ConstructBVHObjectMedian(struct Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index = 0);
void IntersectBVHRecursive(Ray ray, Scene scene, BVH_Node *node, HitData *data, float &tmax, bool &hitAnything);
bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax);