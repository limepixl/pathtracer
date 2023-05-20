#pragma once
#include "../core/array.hpp"
#include "../defines.hpp"
#include "triangle.hpp"
#include <glm/mat4x4.hpp>

struct Mesh
{
    Array<struct Triangle> triangles;
    Array<struct MaterialGLSL> materials;

	glm::mat4 model_matrix {};
    uint32 texture_array {};

    Mesh();

    Mesh(Array<struct Triangle> &triangles,
         Array<struct MaterialGLSL> &materials,
		 glm::mat4 &model_matrix,
         uint32 texture_array);

    Array<TriangleGLSL> ConvertToSSBOFormat();

	void Translate(const glm::vec3 &translation);
	void Rotate(const glm::vec3 &rotation);
	void Scale(const glm::vec3 &scale);
	void Scale(float scale);
    void ApplyModelTransform();
};

