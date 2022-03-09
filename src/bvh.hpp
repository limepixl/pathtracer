#pragma once
#include "defines.hpp"
#include "math.hpp"

/*
	BVH implementation details
*/

struct AABB
{
	Vec3f bmin;
	Vec3f bmax;
};

struct BVH_Node
{
	BVH_Node *left, *right;
	uint32 numTris;

	AABB nodeAABB;

	// TODO: Might be unnecessary
	uint32 index;
};

AABB ConstructAABBFromTris(struct Triangle *tris, int32 numTris);
bool ConstructBVH(struct Triangle *tris, int32 numTris, BVH_Node **node);