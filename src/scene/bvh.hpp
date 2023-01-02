#pragma once
#include "../defines.hpp"
#include "../math/math.hpp"
#include "ray.hpp"
#include "../core/array.hpp"

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

struct BVHNode
{
  AABB node_AABB;
  union
  {
    uint32 left = ~0U;
    uint32 first_tri;
  };
  uint32 num_tris;
  int32 axis;
};

AABB ConstructAABBFromTris(struct Triangle *tris, uint32 num_tris);

bool ConstructBVHSweepSAH(Triangle *tris, uint32 num_tris, Array<BVHNode> &bvh_tree, uint32 bvh_index);

bool ConstructBVHObjectMedian(struct Triangle *tris, uint32 num_tris, Array<BVHNode> &bvh_tree, uint32 bvh_index);

struct BVHNodeGLSL
{
  Vec4f data1; // bmin.x, bmin.y, bmin.z, left/first_tri
  Vec4f data2; // bmax.x, bmax.y, bmax.z, num_tris
  Vec4f data3; // axis, 0, 0, 0
};
