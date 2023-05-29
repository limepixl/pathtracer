#pragma once
#include "../core/array.hpp"
#include "../defines.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Triangle
{
    glm::vec3 v0, v1, v2;
    glm::vec3 n0, n1, n2;
    glm::vec2 uv0, uv1, uv2;

    // Precomputed values
    glm::vec3 edge1, edge2;

    uint32 mat_index;

    Triangle();

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 n0, glm::vec3 n1, glm::vec3 n2, uint32 mat_index);

    Triangle(Array<glm::vec3> &vertices, Array<glm::vec3> &normals, Array<glm::vec2> &tex_coords, uint32 mat_index);
};

struct TriangleGLSL
{
    glm::vec4 data1; // v0.x, v0.y, v0.z, uv0
    glm::vec4 data2; // v1.x, v1.y, v1.z, uv1
    glm::vec4 data3; // v2.x, v2.y, v2.z, uv2
	glm::uvec4 data4; // n0_oct, n1_oct, n2_oct, mat_index

	TriangleGLSL() = default;
	explicit TriangleGLSL(const Triangle &triangle);
	TriangleGLSL(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
				 const glm::vec2 &uv0, const glm::vec2 &uv1, const glm::vec2 &uv2,
				 const glm::vec3 &n0, const glm::vec3 &n1, const glm::vec3 &n2,
				 uint32 mat_index);

	[[nodiscard]] glm::vec3 v0() const;
	[[nodiscard]] glm::vec3 v1() const;
	[[nodiscard]] glm::vec3 v2() const;
};

Array<uint32> FindEmissiveTris(Array<TriangleGLSL> &tris, Array<struct MaterialGLSL> &materials);
