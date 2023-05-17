#include "triangle.hpp"

Triangle::Triangle()
        : v0(0.0f), v1(0.0f), v2(0.0f),
	      n0(0.0f), n1(0.0f), n2(0.0f),
          uv0(0.0f), uv1(0.0f), uv2(0.0f),
          edge1(0.0f), edge2(0.0f), mat_index(0) {}

Triangle::Triangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f n0, Vec3f n1, Vec3f n2, uint32 mat_index)
        : v0(v0), v1(v1), v2(v2),
	      n0(n0), n1(n1), n2(n2),
          uv0(0.0f), uv1(0.0f), uv2(0.0f),
          mat_index(mat_index) {}

Triangle::Triangle(Array<Vec3f> &vertices, Array<Vec3f> &normals, Array<Vec2f> &tex_coords, uint32 mat_index)
        : v0(vertices[0]), v1(vertices[1]), v2(vertices[2]),
	      n0(normals[0]), n1(normals[1]), n2(normals[2]),
          uv0(tex_coords[0]), uv1(tex_coords[1]), uv2(tex_coords[2]),
          mat_index(mat_index)
{}

Vec3f TriangleGLSL::v0() const
{
	return {data1.x, data1.y, data1.z};
}

Vec3f TriangleGLSL::v1() const
{
	return {data2.x, data2.y, data2.z};
}

Vec3f TriangleGLSL::v2() const
{
	return {data3.x, data3.y, data3.z};
}

TriangleGLSL::TriangleGLSL(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2,
			 const Vec2f &uv0, const Vec2f &uv1, const Vec2f &uv2,
			 const Vec3f &n0, const Vec3f &n1, const Vec3f &n2,
			 uint32 mat_index)
	: data1(v0.x, v0.y, v0.z, (float)mat_index),
	  data2(v1.x, v1.y, v1.z, uv0.x),
	  data3(v2.x, v2.y, v2.z, uv0.y),
	  data4(uv1.x, uv1.y, uv2.x, uv2.y),
	  data5(n0.x, n0.y, n0.z, n1.x),
	  data6(n1.y, n1.z, n2.x, n2.y),
	  data7(n2.z, 0.0f, 0.0f, 0.0f)
{}

TriangleGLSL::TriangleGLSL(const Triangle &triangle)
	: TriangleGLSL(triangle.v0, triangle.v1, triangle.v2,
				   triangle.uv0, triangle.uv1, triangle.uv2,
				   triangle.n0, triangle.n1, triangle.n2,
				   triangle.mat_index)
{}
