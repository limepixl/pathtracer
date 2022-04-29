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
	AABB nodeAABB;
	union
	{
		uint32 left = ~0U;
		uint32 first_tri;
	};
	uint32 numTris;
};

AABB ConstructAABBFromTris(struct Triangle *tris, uint32 numTris);
bool ConstructBVHSweepSAH(Triangle *tris, uint32 numTris, Array<BVH_Node> &bvh_tree, uint32 bvh_index);
bool ConstructBVHObjectMedian(struct Triangle *tris, uint32 numTris, Array<BVH_Node> &bvh_tree, uint32 bvh_index);
void IntersectBVHRecursive(Ray ray, Scene scene, BVH_Node *node, HitData *data, float &tmax, bool &hitAnything);
bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax);