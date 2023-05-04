#include "bvh.h"
#include "../math/math.hpp"

#include <vector>

#include <bvh/bvh.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/triangle.hpp>
#include <bvh/vector.hpp>

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &glsl_tris)
{
	bvh::Bvh<float> bvh;

	std::vector<bvh::Triangle<float>> primitives;
	primitives.reserve(glsl_tris.size);

	for (uint32_t i = 0; i < glsl_tris.size; i++)
	{
		const TriangleGLSL &tri = glsl_tris[i];
		bvh::Vector3<float> p0(tri.v0().x, tri.v0().y, tri.v0().z);
		bvh::Vector3<float> p1(tri.v1().x, tri.v1().y, tri.v1().z);
		bvh::Vector3<float> p2(tri.v2().x, tri.v2().y, tri.v2().z);
		primitives.emplace_back(p0, p1, p2);
	}

	// Compute the global bounding box and the centers of the primitives.
	// This is the input of the BVH construction algorithm.
	// Note: Using the bounding box centers instead of the primitive centers is possible,
	// but usually leads to lower-quality BVHs.
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(primitives.data(), primitives.size());
	auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), primitives.size());

	// Create an acceleration data structure on the primitives
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
	builder.build(global_bbox, bboxes.get(), centers.get(), primitives.size());

	Array<BVHNodeGLSL> bvh_nodes;
	for (uint32 i = 0; i < bvh.node_count; i++)
	{
		bvh::Bvh<float>::Node node = bvh.nodes[i];
		bvh::BoundingBox<float> bbox = node.bounding_box_proxy().to_bounding_box();
		Vec3f bmin(bbox.min[0], bbox.min[1], bbox.min[2]);
		Vec3f bmax(bbox.max[0], bbox.max[1], bbox.max[2]);

		BVHNodeGLSL result_node(bmin, bmax, (float)node.first_child_or_primitive, (float)node.primitive_count);
		bvh_nodes.append(result_node);
	}

	return bvh_nodes;
}

BVHNodeGLSL::BVHNodeGLSL(const Vec3f &bmin, const Vec3f &bmax, float first_child_or_tri, float num_tris)
{
	data1 = Vec4f(bmin.x, bmin.y, bmin.z, first_child_or_tri);
	data2 = Vec4f(bmax.x, bmax.y, bmax.z, num_tris);
}
