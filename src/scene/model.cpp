#include "model.h"
#include "material.hpp"
#include <glm/gtc/matrix_transform.hpp>

Model::Model(Array<struct Triangle> &triangles,
           Array<struct MaterialGLSL> &materials,
		   glm::mat4 &model_matrix,
           uint32 texture_array)
        : triangles(triangles),
          materials(materials),
          model_matrix(model_matrix),
          texture_array(texture_array)
{}

Array<TriangleGLSL> Model::ConvertToSSBOFormat()
{
    Array<TriangleGLSL> mesh_tris_ssbo(triangles.size);
    for (uint32 i = 0; i < triangles.size; i++)
    {
        TriangleGLSL glsl_tri(triangles[i]);
        mesh_tris_ssbo.append(glsl_tri);
    }

    return mesh_tris_ssbo;
}

void Model::ApplyModelMatrixToTris()
{
	glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(model_matrix)));
    for(uint32 i = 0; i < triangles.size; i++)
    {
        Triangle &current_tri = triangles[i];
        current_tri.v0 = model_matrix * glm::vec4(current_tri.v0, 1.0f);
        current_tri.v1 = model_matrix * glm::vec4(current_tri.v1, 1.0f);
        current_tri.v2 = model_matrix * glm::vec4(current_tri.v2, 1.0f);

		current_tri.n0 = glm::normalize(normal_matrix * current_tri.n0);
		current_tri.n1 = glm::normalize(normal_matrix * current_tri.n1);
		current_tri.n2 = glm::normalize(normal_matrix * current_tri.n2);
    }
	model_matrix = glm::mat4(1.0f);
}

Model::Model()
	: model_matrix(1.0f), texture_array((uint32) -1)
{}

void Model::Translate(const glm::vec3 &translation)
{
	model_matrix = glm::translate(model_matrix, translation);
}

void Model::Rotate(const glm::vec3 &rotation)
{
	model_matrix = glm::rotate(model_matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model_matrix = glm::rotate(model_matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model_matrix = glm::rotate(model_matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Model::Scale(const glm::vec3 &scale)
{
	model_matrix = glm::scale(model_matrix, scale);
}

void Model::Scale(float scale)
{
	Scale(glm::vec3(scale));
}
