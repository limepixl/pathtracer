#include "bvh.h"
#include "../math/math.hpp"

#include <vector>

#include <bvh/bvh.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/triangle.hpp>
#include <bvh/vector.hpp>

BVHNodeGLSL::BVHNodeGLSL(const Vec3f &bmin, const Vec3f &bmax, uint32 first_child_or_tri, uint32 num_tris)
{
	data1 = Vec4f(bmin.x, bmin.y, bmin.z, (float) first_child_or_tri);
	data2 = Vec4f(bmax.x, bmax.y, bmax.z, (float) num_tris);
}

inline std::vector<bvh::Triangle<float>> ConvertToLibFormat(Array<TriangleGLSL> &tris)
{
	std::vector<bvh::Triangle<float>> primitives;
	primitives.reserve(tris.size);

	for (uint32_t i = 0; i < tris.size; i++)
	{
		const TriangleGLSL &tri = tris[i];
		bvh::Vector3<float> p0(tri.v0().x, tri.v0().y, tri.v0().z);
		bvh::Vector3<float> p1(tri.v1().x, tri.v1().y, tri.v1().z);
		bvh::Vector3<float> p2(tri.v2().x, tri.v2().y, tri.v2().z);
		primitives.emplace_back(p0, p1, p2);
	}

	return primitives;
}

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &glsl_tris, Array<TriangleGLSL> &sorted_glsl_tris)
{
	std::vector<bvh::Triangle<float>> primitives = ConvertToLibFormat(glsl_tris);

	// Compute the global bounding box and the centers of the primitives.
	// This is the input of the BVH construction algorithm.
	// Note: Using the bounding box centers instead of the primitive centers is possible,
	// but usually leads to lower-quality BVHs.
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(primitives.data(), primitives.size());
	auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), primitives.size());

	// Create an acceleration data structure on the primitives
	bvh::Bvh<float> bvh;
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
	builder.build(global_bbox, bboxes.get(), centers.get(), primitives.size());

	Array<BVHNodeGLSL> bvh_nodes;
	for (uint32 i = 0; i < bvh.node_count; i++)
	{
		bvh::Bvh<float>::Node node = bvh.nodes[i];
		bvh::BoundingBox<float> bbox = node.bounding_box_proxy().to_bounding_box();
		Vec3f bmin(bbox.min[0], bbox.min[1], bbox.min[2]);
		Vec3f bmax(bbox.max[0], bbox.max[1], bbox.max[2]);

		BVHNodeGLSL result_node(bmin, bmax, node.first_child_or_primitive, node.primitive_count);
		bvh_nodes.append(result_node);

		if (node.is_leaf())
		{
			for (uint32 j = 0; j < node.primitive_count; j++)
			{
				uint32 index_into_sorted_primitives = node.first_child_or_primitive + j;
				uint32 index_into_unsorted_primitives = (uint32) bvh.primitive_indices[index_into_sorted_primitives];
				TriangleGLSL prev_tri = glsl_tris[index_into_unsorted_primitives];
				sorted_glsl_tris.append(prev_tri);
			}
		}
	}

	printf("Calculated BVH for scene, using %u nodes.\n", bvh_nodes.size);
	return bvh_nodes;
}