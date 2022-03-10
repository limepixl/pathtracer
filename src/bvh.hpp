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

bool operator==(const AABB &lhs, const AABB &rhs);

struct BVH_Node
{
	BVH_Node *left, *right;
	uint32 numTris;
	uint32 index;
	AABB nodeAABB;
};

AABB ConstructAABBFromTris(struct Triangle *tris, int32 numTris);
bool ConstructBVH(struct Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index = 0);
void ApplyModelMatrixToBVH(BVH_Node *node, Mat4f model);