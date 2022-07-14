#include "bvh.hpp"
#include "triangle.hpp"
#include <cmath>
#include <cstdlib>

bool operator==(const AABB &lhs, const AABB &rhs)
{
	return lhs.bmin == rhs.bmin && lhs.bmax == rhs.bmax;
}

AABB ConstructAABBFromTris(Triangle *tris, uint32 num_tris)
{
	Vec3f max_vec = CreateVec3f(INFINITY);
	Vec3f min_vec = CreateVec3f(-INFINITY);
	AABB aabb = { max_vec, min_vec };

	for (uint32 t_index = 0; t_index < num_tris; t_index++)
	{
		Vec3f &v0 = tris[t_index].v0;
		Vec3f &v1 = tris[t_index].v1;
		Vec3f &v2 = tris[t_index].v2;

		aabb.bmin = MinComponentWise(aabb.bmin, v0);
		aabb.bmin = MinComponentWise(aabb.bmin, v1);
		aabb.bmin = MinComponentWise(aabb.bmin, v2);

		aabb.bmax = MaxComponentWise(aabb.bmax, v0);
		aabb.bmax = MaxComponentWise(aabb.bmax, v1);
		aabb.bmax = MaxComponentWise(aabb.bmax, v2);
	}

	// Extend borders of AABB in order to get around
	// situations where the node is flat like a plane
	Vec3f offset_vec = CreateVec3f(0.05f);
	aabb.bmin = aabb.bmin - offset_vec;
	aabb.bmax = aabb.bmax + offset_vec;
	return aabb;
}

static void ExpandAABBWithTri(AABB &aabb, Triangle &tri)
{
	Vec3f offset_vec = CreateVec3f(0.01f);

	Vec3f v0 = tri.v0 - offset_vec;
	Vec3f v1 = tri.v1 - offset_vec;
	Vec3f v2 = tri.v2 - offset_vec;

	aabb.bmin = MinComponentWise(aabb.bmin, v0);
	aabb.bmin = MinComponentWise(aabb.bmin, v1);
	aabb.bmin = MinComponentWise(aabb.bmin, v2);

	v0 = tri.v0 + offset_vec;
	v1 = tri.v1 + offset_vec;
	v2 = tri.v2 + offset_vec;

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
	float tx1 = (aabb.bmin.x - ray.origin.x) * ray.inv_dir.x;
	float tx2 = (aabb.bmax.x - ray.origin.x) * ray.inv_dir.x;

	float tmin = Min(tx1, tx2);
	float tmax = Max(tx1, tx2);

	float ty1 = (aabb.bmin.y - ray.origin.y) * ray.inv_dir.y;
	float ty2 = (aabb.bmax.y - ray.origin.y) * ray.inv_dir.y;

	tmin = Max(tmin, Min(ty1, ty2));
	tmax = Min(tmax, Max(ty1, ty2));

	float tz1 = (aabb.bmin.z - ray.origin.z) * ray.inv_dir.z;
	float tz2 = (aabb.bmax.z - ray.origin.z) * ray.inv_dir.z;

	tmin = Max(tmin, Min(tz1, tz2));
	tmax = Min(tmax, Max(tz1, tz2));

	return tmax >= Max(0.0f, tmin) && tmin < t;
}
/*
bool IntersectBVHStack(Ray ray, Scene scene, HitData *data, float &tmax)
{
	bool hit_anything = false;
	Array<BVHNode> stack = CreateArray<BVHNode>(10);
	AppendToArray(stack, scene.bvh_tree[0]);

	while (stack.size > 0)
	{
		BVHNode node = PopFromArray(stack);

		// Check if ray intersects root node
		bool bvh_hit = AABBIntersect(ray, node.node_AABB, tmax);
		if (bvh_hit)
		{
			bool is_leaf_node = node.num_tris > 0;
			if (!is_leaf_node)
			{
				float dist1 = squaredDist(scene.bvh_tree[node.left].node_AABB, ray.origin);
				float dist2 = squaredDist(scene.bvh_tree[node.left + 1].node_AABB, ray.origin);

				float squared_tmax = tmax * tmax;
				if (dist1 <= dist2)
				{
					if (dist2 < squared_tmax)
						AppendToArray(stack, scene.bvh_tree[node.left + 1]);

					if (dist1 < squared_tmax)
						AppendToArray(stack, scene.bvh_tree[node.left]);
				}
				else
				{
					if (dist1 < squared_tmax)
						AppendToArray(stack, scene.bvh_tree[node.left]);

					if (dist2 < squared_tmax)
						AppendToArray(stack, scene.bvh_tree[node.left + 1]);
				}
			}
			else
			{
				// If leaf node, test against triangles of node
				uint32 num_node_tris = node.num_tris;

				for (uint32 i = 0; i < num_node_tris; i++)
				{
					HitData current_data = {};
					Triangle &current = scene.tris.data[node.first_tri + i];
					bool intersect = TriangleIntersect(ray, current, &current_data, tmax);
					if (intersect && current_data.t < tmax)
					{
						// Found closer hit, store it.
						tmax = current_data.t;
						current_data.object_index = node.first_tri + i;

						hit_anything = true;
						*data = current_data;
					}
				}
			}
		}
	}

	DeallocateArray(stack);
	return hit_anything;
}
*/

static int axis = 0;

// Returns negative integer if a < b, a positive integer if a > b
// and 0 if a == b. Used for qsort in ConstructBVH() below.
static int compare_tris(const void *a, const void *b)
{
	Triangle arg1 = *(const Triangle *)a;
	Triangle arg2 = *(const Triangle *)b;

	if (axis == 0)
	{
		float x_centroid1 = (arg1.v0.x + arg1.v1.x + arg1.v2.x) / 3.0f;
		float x_centroid2 = (arg2.v0.x + arg2.v1.x + arg2.v2.x) / 3.0f;
		if (x_centroid1 < x_centroid2)
			return -1;
		if (x_centroid1 > x_centroid2)
			return 1;

		return 0;
	}
	else if (axis == 1)
	{
		float y_centroid1 = (arg1.v0.y + arg1.v1.y + arg1.v2.y) / 3.0f;
		float y_centroid2 = (arg2.v0.y + arg2.v1.y + arg2.v2.y) / 3.0f;
		if (y_centroid1 < y_centroid2)
			return -1;
		if (y_centroid1 > y_centroid2)
			return 1;

		return 0;
	}
	else
	{
		float z_centroid1 = (arg1.v0.z + arg1.v1.z + arg1.v2.z) / 3.0f;
		float z_centroid2 = (arg2.v0.z + arg2.v1.z + arg2.v2.z) / 3.0f;
		if (z_centroid1 < z_centroid2)
			return -1;
		if (z_centroid1 > z_centroid2)
			return 1;

		return 0;
	}
}

static float SurfaceAreaOfAABB(AABB aabb)
{
	Vec3f dims = aabb.bmax - aabb.bmin;
	// * 2.0f is not needed for comparison purposes
	return (dims.x * dims.y + dims.x * dims.z + dims.y * dims.z);
}

bool ConstructBVHObjectMedian(Triangle *tris, uint32 num_tris, Array<BVHNode> &bvh_tree, uint32 bvh_index)
{
	BVHNode &current_node = bvh_tree[bvh_index];
	current_node.node_AABB = ConstructAABBFromTris(tris, num_tris);

	// If the node has more than n_leaf triangles, it needs to be
	// split into 2 child nodes.
	if (num_tris > BVH_NUM_LEAF_TRIS)
	{
		// Sort triangles along an axis
		qsort(tris, num_tris, sizeof(Triangle), compare_tris);

		// Update axis chosen for next sort
		axis = (axis + 1) % 3;

		// Recurse with the same function for the left and right children
		uint32 left_child_num_tris = num_tris / 2;
		uint32 right_child_num_tris = num_tris % 2 == 0 ? left_child_num_tris : left_child_num_tris + 1;

		BVHNode left_child {};
		left_child.first_tri = current_node.first_tri;
		left_child.num_tris = left_child_num_tris;
		uint32 left_index = (uint32)bvh_tree.size;
		AppendToArray(bvh_tree, left_child);

		BVHNode right_child {};
		right_child.first_tri = current_node.first_tri + left_child_num_tris;
		right_child.num_tris = right_child_num_tris;
		AppendToArray(bvh_tree, right_child);

		current_node.left = left_index;

		ConstructBVHObjectMedian(tris, left_child_num_tris, bvh_tree, left_index);
		ConstructBVHObjectMedian(tris + left_child_num_tris, right_child_num_tris, bvh_tree, left_index + 1);
		current_node.num_tris = 0;
	}
	else
		current_node.num_tris = num_tris;

	return true;
}

bool ConstructBVHSweepSAH(Triangle *tris, uint32 num_tris, Array<BVHNode> &bvh_tree, uint32 bvh_index)
{
	BVHNode &current_node = bvh_tree[bvh_index];
	current_node.node_AABB = ConstructAABBFromTris(tris, num_tris);
	current_node.num_tris = num_tris;
	current_node.axis = -1;

	float S_P = SurfaceAreaOfAABB(current_node.node_AABB);

	if (num_tris > BVH_NUM_LEAF_TRIS)
	{
		// If the node has more than n_leaf triangles, it needs to be
		// split into 2 child nodes.
		float axis_costs[3];
		unsigned int axis_indices[3];

		for (int a = 0; a < 3; a++)
		{
			// Sort triangles along an axis
			axis = a;
			qsort(tris, num_tris, sizeof(Triangle), compare_tris);

			// Compute cost (Sweep SAH method)
			// Cost function:   C(L, R) = ( N(L) * S(L) + N(R) * S(R) ) / S(P)

			Array<float> partition_costs(num_tris - 1);
			partition_costs.size = num_tris - 1;

			AABB V_L, V_R;
			V_L.bmin = { INFINITY, INFINITY, INFINITY };
			V_L.bmax = { -INFINITY, -INFINITY, -INFINITY };
			V_R.bmin = { INFINITY, INFINITY, INFINITY };
			V_R.bmax = { -INFINITY, -INFINITY, -INFINITY };

			// 1) Sweep from right to left to compute (S(R)/S(P)) * N(R)
			for (unsigned int i = 1; i < num_tris; i++)
			{
				Triangle new_tri = tris[num_tris - i];
				ExpandAABBWithTri(V_R, new_tri);
				float S_R = SurfaceAreaOfAABB(V_R);

				float res_R = S_R * (float)i;
				partition_costs[num_tris - i - 1] = res_R;
			}

			// 2) Sweep from left to right to compute full cost (by expanding left node)
			for (unsigned int i = 1; i < num_tris; i++)
			{
				Triangle new_tri = tris[i - 1];
				ExpandAABBWithTri(V_L, new_tri);

				float S_L = SurfaceAreaOfAABB(V_L);
				float res_L = S_L * (float)i;
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

		//// Check if splitting cost is bigger than not splitting
		//uint32 cost_parent = num_tris;
		//if ((float)cost_parent < min_cost)
		//{
		//	// Skipped split of node
		//	printf("Keeping node with %u tris!\n", current_node.num_tris);
		//	return true;
		//}

		current_node.num_tris = 0;

		// Sort triangles along the optimal axis
		if (optimal_axis != 2)
		{
			axis = (int32)optimal_axis;
			qsort(tris, num_tris, sizeof(Triangle), compare_tris);
		}

		// Create left and right child
		uint32 left_child_num_tris = axis_indices[optimal_axis] + 1;
		uint32 right_child_num_tris = num_tris - left_child_num_tris;

		BVHNode left_child {};
		left_child.first_tri = current_node.first_tri;
		left_child.num_tris = left_child_num_tris;
		uint32 left_index = (uint32)bvh_tree.size;
		AppendToArray(bvh_tree, left_child);

		BVHNode right_child {};
		right_child.first_tri = current_node.first_tri + left_child_num_tris;
		right_child.num_tris = right_child_num_tris;
		AppendToArray(bvh_tree, right_child);

		current_node.left = left_index;
		current_node.axis = (int16)axis;

		// printf("Left child #tris: %d\n", left_child_num_tris);
		// printf("Right child #tris: %d\n", right_child_num_tris);
		// printf("/////////////////////////////////\n");

		// Recurse with the same function for the left and right children
		ConstructBVHSweepSAH(tris, left_child_num_tris, bvh_tree, current_node.left);
		ConstructBVHSweepSAH(tris + left_child_num_tris, right_child_num_tris, bvh_tree, current_node.left + 1);
	}

	return true;
}
