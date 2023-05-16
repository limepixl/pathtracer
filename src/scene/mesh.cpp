#include "mesh.h"
#include "material.hpp"

Mesh::Mesh(Array<struct Triangle> &triangles,
           Array<struct MaterialGLSL> &materials,
           Mat4f &model_matrix,
           uint32 texture_array)
        : triangles(triangles),
          materials(materials),
          model_matrix(model_matrix),
          texture_array(texture_array)
{

}

Array<TriangleGLSL> Mesh::ConvertToSSBOFormat()
{
    Array<TriangleGLSL> mesh_tris_ssbo(triangles.size);
    for (uint32 i = 0; i < triangles.size; i++)
    {
        TriangleGLSL glsl_tri(triangles[i]);
        mesh_tris_ssbo.append(glsl_tri);
    }

    return mesh_tris_ssbo;
}

void Mesh::ApplyModelTransform()
{
    for(uint32 i = 0; i < triangles.size; i++)
    {
        Triangle &current_tri = triangles[i];
        current_tri.v0 = model_matrix * current_tri.v0;
        current_tri.v1 = model_matrix * current_tri.v1;
        current_tri.v2 = model_matrix * current_tri.v2;
    }
}
