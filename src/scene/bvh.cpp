#include "bvh.hpp"
#include "triangle.hpp"
#include <cstdlib>
#include <math.h>

bool operator==(const AABB &lhs, const AABB &rhs)
{
	return lhs.bmin == rhs.bmin && lhs.bmax == rhs.bmax;
}

AABB ConstructAABBFromTris(Triangle *tris, uint32 numTris)
{
	Vec3f maxVec = CreateVec3f(INFINITY);
	Vec3f minVec = CreateVec3f(-INFINITY);
	AABB aabb = {maxVec, minVec};

	for(uint32 tIndex = 0; tIndex < numTris; tIndex++)
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

	// Extend borders of AABB in order to get around 
	// situations where the node is flat like a plane
	Vec3f offsetVec = CreateVec3f(0.05f);
	aabb.bmin = aabb.bmin - offsetVec;
	aabb.bmax = aabb.bmax + offsetVec;
	return aabb;
}

// Returns -1 if first is closer than second
// Returns  1 if second is closer than first
// Returns  0 if both are overlapping (should not happen)
int CloserAABB(AABB first, AABB second, Ray ray)
{
	Vec3f point1 = ray.origin;

	// Clamp the point to the first AABB
	if(point1.x > first.bmax.x)
		point1.x = first.bmax.x;
	else if(point1.x < first.bmin.x)
		point1.x = first.bmin.x;
	if(point1.y > first.bmax.y)
		point1.y = first.bmax.y;
	else if(point1.y < first.bmin.y)
		point1.y = first.bmin.y;
	if(point1.z > first.bmax.z)
		point1.z = first.bmax.z;
	else if(point1.z < first.bmin.z)
		point1.z = first.bmin.z;

	Vec3f point2 = ray.origin;

	// Clamp the point to the second AABB
	if(point2.x > second.bmax.x)
		point2.x = second.bmax.x;
	else if(point2.x < second.bmin.x)
		point2.x = second.bmin.x;
	if(point2.y > second.bmax.y)
		point2.y = second.bmax.y;
	else if(point2.y < second.bmin.y)
		point2.y = second.bmin.y;
	if(point2.z > second.bmax.z)
		point2.z = second.bmax.z;
	else if(point2.z < second.bmin.z)
		point2.z = second.bmin.z;

	point1 = point1 - ray.origin;
	point2 = point2 - ray.origin;

	// Compare squared lengths
	float sl1 = Dot(point1, point1);
	float sl2 = Dot(point2, point2);
	if(sl1 < sl2) return -1;
	if(sl1 > sl2) return 1;
	return 0;
}

// Slab method
// https://tavianator.com/2011/ray_box.html
bool AABBIntersect(Ray ray, AABB aabb, float t)
{
	Vec3f invDir = 1.0f / ray.direction;

	float tx1 = (aabb.bmin.x - ray.origin.x)*invDir.x;
	float tx2 = (aabb.bmax.x - ray.origin.x)*invDir.x;

	float tmin = Min(tx1, tx2);
	float tmax = Max(tx1, tx2);

	float ty1 = (aabb.bmin.y - ray.origin.y)*invDir.y;
	float ty2 = (aabb.bmax.y - ray.origin.y)*invDir.y;

	tmin = Max(tmin, Min(ty1, ty2));
	tmax = Min(tmax, Max(ty1, ty2));

	float tz1 = (aabb.bmin.z - ray.origin.z)*invDir.z;
	float tz2 = (aabb.bmax.z - ray.origin.z)*invDir.z;

	tmin = Max(tmin, Min(tz1, tz2));
	tmax = Min(tmax, Max(tz1, tz2));

	return tmax >= Max(0.0f, tmin) && tmin < t;
/*
	// First check if ray origin is within AABB
	Vec3f &ro = ray.origin;
	if(ro > aabb.bmin && ro < aabb.bmax)
		return true;

	float t0x, t1x, t0y, t1y, t0z, t1z;
	Vec3f inverseDir = 1.0f / ray.direction;
	
	// X axis interval
	if(inverseDir.x >= 0)
	{
		t0x = (aabb.bmin.x - ray.origin.x) * inverseDir.x;
		t1x = (aabb.bmax.x - ray.origin.x) * inverseDir.x;
	}
	else
	{
		t0x = (aabb.bmax.x - ray.origin.x) * inverseDir.x;
		t1x = (aabb.bmin.x - ray.origin.x) * inverseDir.x;
	}
	
	// Y axis interval
	if(inverseDir.y >= 0)
	{
		t0y = (aabb.bmin.y - ray.origin.y) * inverseDir.y;
		t1y = (aabb.bmax.y - ray.origin.y) * inverseDir.y;
	}
	else
	{
		t0y = (aabb.bmax.y - ray.origin.y) * inverseDir.y;
		t1y = (aabb.bmin.y - ray.origin.y) * inverseDir.y;
	}

	if(t0x > t1y || t0y > t1x)
		return false;

	float tmin_t = Max(t0x, t0y);
	float tmax_t = Min(t1x, t1y);

	// Z axis interval
	if(inverseDir.z >= 0)
	{
		t0z = (aabb.bmin.z - ray.origin.z) * inverseDir.z;
		t1z = (aabb.bmax.z - ray.origin.z) * inverseDir.z;
	}
	else
	{
		t0z = (aabb.bmax.z - ray.origin.z) * inverseDir.z;
		t1z = (aabb.bmin.z - ray.origin.z) * inverseDir.z;
	}

	if(tmin_t > t1z || t0z > tmax_t)
		return false;

	tmin_t = Max(tmin_t, t0z);
	tmax_t = Min(tmax_t, t1z);

	if(tmin_t <= TMIN || tmin_t >= t)
		return false;

	return true;
*/
}

void IntersectBVH(Ray ray, Scene scene, BVH_Node *node, HitData *data, float &tmax, bool &hitAnything)
{
	// Check if there is an intersection with the current node
	bool bvhHit = AABBIntersect(ray, node->nodeAABB, tmax);
	if(bvhHit)
	{
		bool isLeafNode = node->numTris <= BVH_NUM_LEAF_TRIS;
		if(!isLeafNode)
		{
			// Check which child is closer
			int res = CloserAABB(node->left->nodeAABB, node->right->nodeAABB, ray);
			
			if(res == -1 || res == 0)
			{
				IntersectBVH(ray, scene, node->left, data, tmax, hitAnything);
				IntersectBVH(ray, scene, node->right, data, tmax, hitAnything);
			}
			else if(res == 1)
			{
				IntersectBVH(ray, scene, node->right, data, tmax, hitAnything);
				IntersectBVH(ray, scene, node->left, data, tmax, hitAnything);
			}
		}
		else
		{
			// If leaf node, test against triangles of node
			Triangle *nodeTris = scene.tris + node->index;
			uint32 numNodeTris = node->numTris;
			
			for(uint32 i = 0; i < numNodeTris; i++)
			{
				HitData currentData = {};
				Triangle *current = &nodeTris[i];
				bool intersect = TriangleIntersect(ray, current, &currentData, tmax);
				if(intersect && currentData.t < tmax)
				{
					// Found closer hit, store it.
					tmax = currentData.t;
					currentData.objectIndex = node->index + i;

					hitAnything = true;
					*data = currentData;
				}
			}
		}
	}
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
		float xCentroid1 = (arg1.v0.x + arg1.v1.x + arg1.v2.x) / 3.0f;
		float xCentroid2 = (arg2.v0.x + arg2.v1.x + arg2.v2.x) / 3.0f;
		if(xCentroid1 < xCentroid2) 
			return -1;
		if(xCentroid1 > xCentroid2)
			return 1;

		return 0;
	}
	else if(axis == 1)
	{
		float yCentroid1 = (arg1.v0.y + arg1.v1.y + arg1.v2.y) / 3.0f;
		float yCentroid2 = (arg2.v0.y + arg2.v1.y + arg2.v2.y) / 3.0f;
		if(yCentroid1 < yCentroid2) 
			return -1;
		if(yCentroid1 > yCentroid2)
			return 1;

		return 0;
	}
	else
	{
		float zCentroid1 = (arg1.v0.z + arg1.v1.z + arg1.v2.z) / 3.0f;
		float zCentroid2 = (arg2.v0.z + arg2.v1.z + arg2.v2.z) / 3.0f;
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

	current_node->nodeAABB = ConstructAABBFromTris(tris, numTris);

	return true;
}