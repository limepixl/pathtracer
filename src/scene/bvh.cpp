#include "bvh.hpp"
#include "triangle.hpp"
#include <cmath>
#include <cstdlib>

bool operator==(const AABB &lhs, const AABB &rhs)
{
	return lhs.bmin == rhs.bmin && lhs.bmax == rhs.bmax;
}

AABB ConstructAABBFromTris(Triangle *tris, uint32 numTris)
{
	Vec3f maxVec = CreateVec3f(INFINITY);
	Vec3f minVec = CreateVec3f(-INFINITY);
	AABB aabb = { maxVec, minVec };

	for (uint32 tIndex = 0; tIndex < numTris; tIndex++)
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

void ExpandAABBWithTri(AABB &aabb, Triangle &tri)
{
	Vec3f offsetVec = CreateVec3f(0.01f);

	Vec3f v0 = tri.v0 - offsetVec;
	Vec3f v1 = tri.v1 - offsetVec;
	Vec3f v2 = tri.v2 - offsetVec;

	aabb.bmin = MinComponentWise(aabb.bmin, v0);
	aabb.bmin = MinComponentWise(aabb.bmin, v1);
	aabb.bmin = MinComponentWise(aabb.bmin, v2);

	v0 = tri.v0 + offsetVec;
	v1 = tri.v1 + offsetVec;
	v2 = tri.v2 + offsetVec;

	aabb.bmax = MaxComponentWise(aabb.bmax, v0);
	aabb.bmax = MaxComponentWise(aabb.bmax, v1);
	aabb.bmax = MaxComponentWise(aabb.bmax, v2);
}

inline float squaredDist(AABB &aabb, Vec3f &point)
{
	float dx = Max(Max(aabb.bmin.x - point.x, 0.0f), point.x - aabb.bmax.x);
	float dy = Max(Max(aabb.bmin.y - point.y, 0.0f), point.y - aabb.bmax.y);
	float dz = Max(Max(aabb.bmin.z - point.z, 0.0f), point.z - aabb.bmax.z);
	return dx * dx + dy * dy + dz * dz;
}

// Slab method
// https://tavianator.com/2011/ray_box.html
bool AABBIntersect(Ray ray, AABB aabb, float t)
{
	float tx1 = (aabb.bmin.x - ray.origin.x) * ray.invDir.x;
	float tx2 = (aabb.bmax.x - ray.origin.x) * ray.invDir.x;

	float tmin = Min(tx1, tx2);
	float tmax = Max(tx1, tx2);

	float ty1 = (aabb.bmin.y - ray.origin.y) * ray.invDir.y;
	float ty2 = (aabb.bmax.y - ray.origin.y) * ray.invDir.y;

	tmin = Max(tmin, Min(ty1, ty2));
	tmax = Min(tmax, Max(ty1, ty2));

	float tz1 = (aabb.bmin.z - ray.origin.z) * ray.invDir.z;
	float tz2 = (aabb.bmax.z - ray.origin.z) * ray.invDir.z;

	tmin = Max(tmin, Min(tz1, tz2));
	tmax = Min(tmax, Max(tz1, tz2));

	return tmax >= Max(0.0f, tmin) && tmin < t;
}

float AABBIntersectValue(Ray ray, AABB aabb, float t)
{
	float tx1 = (aabb.bmin.x - ray.origin.x) * ray.invDir.x;
	float tx2 = (aabb.bmax.x - ray.origin.x) * ray.invDir.x;

	float tmin = Min(tx1, tx2);
	float tmax = Max(tx1, tx2);

	float ty1 = (aabb.bmin.y - ray.origin.y) * ray.invDir.y;
	float ty2 = (aabb.bmax.y - ray.origin.y) * ray.invDir.y;

	tmin = Max(tmin, Min(ty1, ty2));
	tmax = Min(tmax, Max(ty1, ty2));

	float tz1 = (aabb.bmin.z - ray.origin.z) * ray.invDir.z;
	float tz2 = (aabb.bmax.z - ray.origin.z) * ray.invDir.z;

	tmin = Max(tmin, Min(tz1, tz2));
	tmax = Min(tmax, Max(tz1, tz2));

	if (tmax >= Max(0.0f, tmin) && tmin < t)
		return tmin;

	return t;
}

bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax)
{
	bool hitAnything = false;
	Array<BVH_Node> stack = CreateArray<BVH_Node>(10);
	AppendToArray(stack, scene.bvh_tree[0]);

	while (stack.size > 0)
	{
		BVH_Node node = PopFromArray(stack);

		// Check if ray intersects root node
		bool bvhHit = AABBIntersect(ray, node.nodeAABB, tmax);
		if (bvhHit)
		{
			bool isLeafNode = node.numTris > 0;
			if (!isLeafNode)
			{
				float dist1 = squaredDist(scene.bvh_tree[node.left].nodeAABB, ray.origin);
				float dist2 = squaredDist(scene.bvh_tree[node.left + 1].nodeAABB, ray.origin);

				float squaredtmax = tmax * tmax;
				if (dist1 <= dist2)
				{
					if (dist2 < squaredtmax)
						AppendToArray(stack, scene.bvh_tree[node.left + 1]);

					if (dist1 < squaredtmax)
						AppendToArray(stack, scene.bvh_tree[node.left]);
				}
				else
				{
					if (dist1 < squaredtmax)
						AppendToArray(stack, scene.bvh_tree[node.left]);

					if (dist2 < squaredtmax)
						AppendToArray(stack, scene.bvh_tree[node.left + 1]);
				}
			}
			else
			{
				// If leaf node, test against triangles of node
				uint32 numNodeTris = node.numTris;

				for (uint32 i = 0; i < numNodeTris; i++)
				{
					HitData currentData = {};
					Triangle &current = scene.tris.data[node.first_tri + i];
					bool intersect = TriangleIntersect(ray, current, &currentData, tmax);
					if (intersect && currentData.t < tmax)
					{
						// Found closer hit, store it.
						tmax = currentData.t;
						currentData.objectIndex = node.first_tri + i;

						hitAnything = true;
						*data = currentData;
					}
				}
			}
		}
	}

	DeallocateArray(stack);
	return hitAnything;
}

static int axis = 0;

// Returns negative integer if a < b, a positive integer if a > b
// and 0 if a == b. Used for qsort in ConstructBVH() below.
int compare_tris(const void *a, const void *b)
{
	Triangle arg1 = *(const Triangle *)a;
	Triangle arg2 = *(const Triangle *)b;

	if (axis == 0)
	{
		float xCentroid1 = (arg1.v0.x + arg1.v1.x + arg1.v2.x) / 3.0f;
		float xCentroid2 = (arg2.v0.x + arg2.v1.x + arg2.v2.x) / 3.0f;
		if (xCentroid1 < xCentroid2)
			return -1;
		if (xCentroid1 > xCentroid2)
			return 1;

		return 0;
	}
	else if (axis == 1)
	{
		float yCentroid1 = (arg1.v0.y + arg1.v1.y + arg1.v2.y) / 3.0f;
		float yCentroid2 = (arg2.v0.y + arg2.v1.y + arg2.v2.y) / 3.0f;
		if (yCentroid1 < yCentroid2)
			return -1;
		if (yCentroid1 > yCentroid2)
			return 1;

		return 0;
	}
	else
	{
		float zCentroid1 = (arg1.v0.z + arg1.v1.z + arg1.v2.z) / 3.0f;
		float zCentroid2 = (arg2.v0.z + arg2.v1.z + arg2.v2.z) / 3.0f;
		if (zCentroid1 < zCentroid2)
			return -1;
		if (zCentroid1 > zCentroid2)
			return 1;

		return 0;
	}
}

float SurfaceAreaOfAABB(AABB aabb)
{
	Vec3f dims = aabb.bmax - aabb.bmin;
	// * 2.0f is not needed for comparison purposes
	return (dims.x * dims.y + dims.x * dims.z + dims.y * dims.z);
}

bool ConstructBVHObjectMedian(Triangle *tris, uint32 numTris, Array<BVH_Node> &bvh_tree, uint32 bvh_index)
{
	BVH_Node &current_node = bvh_tree[bvh_index];
	current_node.nodeAABB = ConstructAABBFromTris(tris, numTris);

	// If the node has more than n_leaf triangles, it needs to be
	// split into 2 child nodes.
	if (numTris > BVH_NUM_LEAF_TRIS)
	{
		// Sort triangles along an axis
		qsort(tris, numTris, sizeof(Triangle), compare_tris);

		// Update axis chosen for next sort
		axis = (axis + 1) % 3;

		// Recurse with the same function for the left and right children
		uint32 leftChildNumTris = numTris / 2;
		uint32 rightChildNumTris = numTris % 2 == 0 ? leftChildNumTris : leftChildNumTris + 1;

		BVH_Node leftChild {};
		leftChild.first_tri = current_node.first_tri;
		leftChild.numTris = leftChildNumTris;
		uint32 left_index = bvh_tree.size;
		AppendToArray(bvh_tree, leftChild);

		BVH_Node rightChild {};
		rightChild.first_tri = current_node.first_tri + leftChildNumTris;
		rightChild.numTris = rightChildNumTris;
		AppendToArray(bvh_tree, rightChild);

		current_node.left = left_index;

		ConstructBVHObjectMedian(tris, leftChildNumTris, bvh_tree, left_index);
		ConstructBVHObjectMedian(tris + leftChildNumTris, rightChildNumTris, bvh_tree, left_index + 1);
		current_node.numTris = 0;
	}
	else
		current_node.numTris = numTris;

	return true;
}

bool ConstructBVHSweepSAH(Triangle *tris, uint32 numTris, Array<BVH_Node> &bvh_tree, uint32 bvh_index)
{
	BVH_Node &current_node = bvh_tree[bvh_index];
	current_node.nodeAABB = ConstructAABBFromTris(tris, numTris);
	current_node.numTris = numTris;

	float S_P = SurfaceAreaOfAABB(current_node.nodeAABB);

	if (numTris > BVH_NUM_LEAF_TRIS)
	{
		// If the node has more than n_leaf triangles, it needs to be
		// split into 2 child nodes.
		float axis_costs[3];
		unsigned int axis_indices[3];

		for (int a = 0; a < 3; a++)
		{
			// Sort triangles along an axis
			axis = a;
			qsort(tris, numTris, sizeof(Triangle), compare_tris);

			// Compute cost (Sweep SAH method)
			// Cost function:   C(L, R) = ( N(L) * S(L) + N(R) * S(R) ) / S(P)

			Array<float> partition_costs = CreateArray<float>(numTris - 1);
			partition_costs.size = numTris - 1;

			AABB V_L, V_R;
			V_L.bmin = { INFINITY, INFINITY, INFINITY };
			V_L.bmax = { -INFINITY, -INFINITY, -INFINITY };
			V_R.bmin = { INFINITY, INFINITY, INFINITY };
			V_R.bmax = { -INFINITY, -INFINITY, -INFINITY };

			// 1) Sweep from right to left to compute (S(R)/S(P)) * N(R)
			for (unsigned int i = 1; i < numTris; i++)
			{
				Triangle newTri = tris[numTris - i];
				ExpandAABBWithTri(V_R, newTri);
				float S_R = SurfaceAreaOfAABB(V_R);

				float res_R = S_R * i;
				partition_costs[numTris - i - 1] = res_R;
			}

			// 2) Sweep from left to right to compute full cost (by expanding left node)
			for (unsigned int i = 1; i < numTris; i++)
			{
				Triangle newTri = tris[i - 1];
				ExpandAABBWithTri(V_L, newTri);

				float S_L = SurfaceAreaOfAABB(V_L);
				float res_L = S_L * i;
				partition_costs[i - 1] += res_L;
			}

			// 3) Keep only the split with the minimum cost
			float min_cost = INFINITY;
			unsigned int min_index = 0;
			for (unsigned int i = 0; i < partition_costs.size; i++)
			{
				partition_costs[i] /= S_P;

				if (partition_costs[i] < 0)
				{
					printf("//////////////////////////////////////////////////////////////////////////////////////////////////////////\n");
				}
				if (partition_costs[i] < min_cost)
				{
					min_cost = partition_costs[i];
					min_index = i;
				}
			}

			DeallocateArray(partition_costs);

			axis_costs[a] = min_cost;
			axis_indices[a] = min_index;
		}

		float min_cost = axis_costs[0];
		unsigned int optimal_axis = 0;

		if (axis_costs[1] < min_cost)
		{
			min_cost = axis_costs[1];
			optimal_axis = 1;
		}
		if (axis_costs[2] < min_cost)
		{
			optimal_axis = 2;
		}

		// Check if splitting cost is bigger than not splitting
		uint32 cost_parent = numTris;
		if ((float)cost_parent < min_cost)
		{
			// Skipped split of node
			// printf("Keeping node with %u tris!\n", current_node.numTris);
			return true;
		}

		current_node.numTris = 0;

		// Sort triangles along the optimal axis
		if (optimal_axis != 2)
		{
			axis = optimal_axis;
			qsort(tris, numTris, sizeof(Triangle), compare_tris);
		}

		// Create left and right child
		uint32 leftChildNumTris = axis_indices[optimal_axis] + 1;
		uint32 rightChildNumTris = numTris - leftChildNumTris;

		BVH_Node leftChild {};
		leftChild.first_tri = current_node.first_tri;
		leftChild.numTris = leftChildNumTris;
		uint32 left_index = bvh_tree.size;
		AppendToArray(bvh_tree, leftChild);

		BVH_Node rightChild {};
		rightChild.first_tri = current_node.first_tri + leftChildNumTris;
		rightChild.numTris = rightChildNumTris;
		AppendToArray(bvh_tree, rightChild);

		current_node.left = left_index;

		// printf("Left child #tris: %d\n", leftChildNumTris);
		// printf("Right child #tris: %d\n", rightChildNumTris);
		// printf("/////////////////////////////////\n");

		// Recurse with the same function for the left and right children
		ConstructBVHSweepSAH(tris, leftChildNumTris, bvh_tree, current_node.left);
		ConstructBVHSweepSAH(tris + leftChildNumTris, rightChildNumTris, bvh_tree, current_node.left + 1);
	}

	return true;
}