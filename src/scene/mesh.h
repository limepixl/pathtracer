#pragma once
#include "../math/mat4.hpp"
#include "../core/array.hpp"
#include "../defines.hpp"

struct Mesh
{
	Array<struct Triangle> triangles;
	Array<struct MaterialGLSL> materials;

	Mat4f model_matrix;
	uint32 texture_array{};
	uint32 texture_unit{};

	Mesh() = default;

	Mesh(Array<struct Triangle> &triangles,
		 Array<struct MaterialGLSL> &materials,
		 Mat4f &model_matrix,
		 uint32 texture_array,
		 uint32 texture_unit);
};
