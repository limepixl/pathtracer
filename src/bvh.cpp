#include "bvh.hpp"
#include "intersect.hpp"
#include <vector>
#include <cstdlib>

bool operator==(const AABB &lhs, const AABB &rhs)
{
	return lhs.bmin == rhs.bmin && lhs.bmax == rhs.bmax;
}

AABB ConstructAABBFromTris(Triangle *tris, int32 numTris)
{
	Vec3f maxVec = CreateVec3f(INFINITY);
	Vec3f minVec = CreateVec3f(-INFINITY);
	AABB aabb = {maxVec, minVec};

	for(int32 tIndex = 0; tIndex < numTris; tIndex++)
	{
		Vec3f &v0 = tris[tIndex].v0;
		Vec3f &v1 = tris[tIndex].v1;
		Vec3f &v2 = tris[tIndex].v2;

		aabb.bmin = MinComponentWise(aabb.bmin, v0);
		aabb.bmin = MinComponentWise(aabb.bmin, v1);
		aabb.bmin = MinComponentWise(aabb.bmin, v2);

		aabb.bmax = MaxComponentWise(aabb.bmax, v0);
		aabb.bmax = MaxComponentWise(aabb.bmax, v1);
		aabb.bmax = MaxComponentWise(aabb.bmax, v2);
	}

	aabb.bmin.x -= 0.05f; aabb.bmin.y -= 0.05f; aabb.bmin.z -= 0.05f;
	aabb.bmax.x += 0.05f; aabb.bmax.y += 0.05f; aabb.bmax.z += 0.05f;
	return aabb;
}

static int axis = 0;

// Returns negative integer if a < b, a positive integer if a > b
// and 0 if a == b. Used for qsort in ConstructBVH() below.
int compare_tris(const void* a, const void* b)
{
    Triangle arg1 = *(const Triangle *)a;
    Triangle arg2 = *(const Triangle *)b;

	if(axis == 0)
	{
		float32 xCentroid1 = (arg1.v0.x + arg1.v1.x + arg1.v2.x) / 3.0f;
		float32 xCentroid2 = (arg2.v0.x + arg2.v1.x + arg2.v2.x) / 3.0f;
		if(xCentroid1 < xCentroid2) 
			return -1;
		if(xCentroid1 > xCentroid2)
			return 1;

		return 0;
	}
	else if(axis == 1)
	{
		float32 yCentroid1 = (arg1.v0.y + arg1.v1.y + arg1.v2.y) / 3.0f;
		float32 yCentroid2 = (arg2.v0.y + arg2.v1.y + arg2.v2.y) / 3.0f;
		if(yCentroid1 < yCentroid2) 
			return -1;
		if(yCentroid1 > yCentroid2)
			return 1;

		return 0;
	}
	else
	{
		float32 zCentroid1 = (arg1.v0.z + arg1.v1.z + arg1.v2.z) / 3.0f;
		float32 zCentroid2 = (arg2.v0.z + arg2.v1.z + arg2.v2.z) / 3.0f;
		if(zCentroid1 < zCentroid2) 
			return -1;
		if(zCentroid1 > zCentroid2)
			return 1;

		return 0;
	}
}

bool ConstructBVH(Triangle *tris, uint32 numTris, BVH_Node **node, uint32 index)
{
	*node = new BVH_Node();
	BVH_Node *current_node = *node;

	current_node->left = NULL;
	current_node->right = NULL;
	current_node->index = index;
	current_node->numTris = numTris;
	current_node->nodeAABB = ConstructAABBFromTris(tris, numTris);

	// If the node has more than n_leaf triangles, it needs to be
	// split into 2 child nodes.
	if(numTris > BVH_NUM_LEAF_TRIS)
	{
		// Sort triangles along an axis
		qsort(tris, numTris, sizeof(Triangle), compare_tris);

		// Update axis chosen for next sort
		axis = (axis + 1) % 3;

		// Recurse with the same function for the left and right children
		uint32 leftChildNumTris = numTris / 2;
		uint32 rightChildNumTris = numTris % 2 == 0 ? leftChildNumTris : leftChildNumTris + 1;
		ConstructBVH(tris, leftChildNumTris, &current_node->left, index);
		ConstructBVH(tris + leftChildNumTris, rightChildNumTris, &current_node->right, index + leftChildNumTris);
	}

	return true;
}

void ApplyModelMatrixToBVH(BVH_Node *node, Mat4f model)
{
	node->nodeAABB.bmin = model * node->nodeAABB.bmin;
	node->nodeAABB.bmax = model * node->nodeAABB.bmax;
	
	if(node->left != NULL)
		ApplyModelMatrixToBVH(node->left, model);

	if(node->right != NULL)
		ApplyModelMatrixToBVH(node->right, model);
}