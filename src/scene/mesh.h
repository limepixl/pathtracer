#pragma once
#include "../math/mat4.hpp"
#include "../core/array.hpp"
#include "../defines.hpp"

struct Mesh
{
	Mat4f model_matrix;
	Array<struct Triangle> triangles;

	uint32 texture_array{};
	uint32 texture_unit{};
	Array<struct MaterialGLSL> materials;

	Mesh() = default;

	Mesh(Array<struct Triangle> &triangles,
		 Array<struct MaterialGLSL> &materials,
		 Mat4f &model_matrix,
		 uint32 texture_array,
		 uint32 texture_unit);
};
