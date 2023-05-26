#include "triangle.hpp"
#include "../math/math.hpp"
#include <glm/packing.hpp>

Triangle::Triangle()
        : v0(0.0f), v1(0.0f), v2(0.0f),
	      n0(0.0f), n1(0.0f), n2(0.0f),
          uv0(0.0f), uv1(0.0f), uv2(0.0f),
          edge1(0.0f), edge2(0.0f), mat_index(0) {}

Triangle::Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 n0, glm::vec3 n1, glm::vec3 n2, uint32 mat_index)
        : v0(v0), v1(v1), v2(v2),
	      n0(n0), n1(n1), n2(n2),
          uv0(0.0f), uv1(0.0f), uv2(0.0f),
		  edge1(v1 - v0), edge2(v2 - v0),
          mat_index(mat_index) {}

Triangle::Triangle(Array<glm::vec3> &vertices, Array<glm::vec3> &normals, Array<glm::vec2> &tex_coords, uint32 mat_index)
        : v0(vertices[0]), v1(vertices[1]), v2(vertices[2]),
	      n0(normals[0]), n1(normals[1]), n2(normals[2]),
	      uv0(0.0f), uv1(0.0f), uv2(0.0f),
	      edge1(v1 - v0), edge2(v2 - v0),
          mat_index(mat_index)
{
	if(tex_coords.size == 3)
	{
		uv0 = tex_coords[0];
		uv1 = tex_coords[1];
		uv2 = tex_coords[2];
	}
}

glm::vec3 TriangleGLSL::v0() const
{
	return {data1.x, data1.y, data1.z};
}

glm::vec3 TriangleGLSL::v1() const
{
	return {data2.x, data2.y, data2.z};
}

glm::vec3 TriangleGLSL::v2() const
{
	return {data3.x, data3.y, data3.z};
}

TriangleGLSL::TriangleGLSL(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
			 const glm::vec2 &uv0, const glm::vec2 &uv1, const glm::vec2 &uv2,
			 const glm::vec3 &n0, const glm::vec3 &n1, const glm::vec3 &n2,
			 uint32 mat_index)
	: data1(v0.x, v0.y, v0.z, (float)mat_index),
	  data2(v1.x, v1.y, v1.z, uv0.x),
	  data3(v2.x, v2.y, v2.z, uv0.y),
	  data4(uv1.x, uv1.y, uv2.x, uv2.y),
	  data5(0)
{
    glm::vec2 n0_enc = pixl::octahedral_normal_encoding(n0);
    glm::vec2 n1_enc = pixl::octahedral_normal_encoding(n1);
    glm::vec2 n2_enc = pixl::octahedral_normal_encoding(n2);

	data5.x = glm::packHalf2x16(n0_enc);
	data5.y = glm::packHalf2x16(n1_enc);
	data5.z = glm::packHalf2x16(n2_enc);
}

TriangleGLSL::TriangleGLSL(const Triangle &triangle)
	: TriangleGLSL(triangle.v0, triangle.v1, triangle.v2,
				   triangle.uv0, triangle.uv1, triangle.uv2,
				   triangle.n0, triangle.n1, triangle.n2,
				   triangle.mat_index)
{}
