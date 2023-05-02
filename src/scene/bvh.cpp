#include "bvh.h"

#include <vector>

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>

Array<BVHNodeGLSL> CalculateBVH(Array<TriangleGLSL> &triangles)
{
	bvh::Bvh<float> bvh;

	std::vector<bvh::Triangle<float>> tris;
	tris.reserve(triangles.size);

	for(uint32_t i = 0; i < triangles.size; i++)
	{
		const TriangleGLSL &tmp_tri = triangles[i];
		bvh::Vector3<float> p0(tmp_tri.data1.x, tmp_tri.data1.y, tmp_tri.data1.z);
		bvh::Vector3<float> p1(tmp_tri.data2.x, tmp_tri.data2.y, tmp_tri.data2.z);
		bvh::Vector3<float> p2(tmp_tri.data3.x, tmp_tri.data3.y, tmp_tri.data3.z);
		tris.emplace_back(p0, p1, p2);
	}

    // Compute the global bounding box and the centers of the primitives.
    // This is the input of the BVH construction algorithm.
    // Note: Using the bounding box centers instead of the primitive centers is possible,
    // but usually leads to lower-quality BVHs.
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(tris.data(), tris.size());
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), tris.size());

    // Create an acceleration data structure on the primitives
    bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
    builder.build(global_bbox, bboxes.get(), centers.get(), tris.size());

	Array<BVHNodeGLSL> bvh_nodes;
	for(uint32 i = 0; i < bvh.node_count; i++)
	{
		auto node = bvh.nodes[i];
		Vec3f bmin(node.bounds[0], node.bounds[2], node.bounds[4]);
		Vec3f bmax(node.bounds[1], node.bounds[3], node.bounds[5]);

		BVHNodeGLSL result_node{};
		result_node.data1 = Vec4f(bmin.x, bmin.y, bmin.z, (float) node.first_child_or_primitive);
		result_node.data2 = Vec4f(bmax.x, bmax.y, bmax.z, (float) node.primitive_count);
		bvh_nodes.append(result_node);
	}

	return bvh_nodes;
}