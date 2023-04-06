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
        const Triangle &current_tri = triangles[i];

        const Vec3f &v0 = current_tri.v0;
        const Vec3f &v1 = current_tri.v1;
        const Vec3f &v2 = current_tri.v2;
        const Vec2f &uv0 = current_tri.uv0;
        const Vec2f &uv1 = current_tri.uv1;
        const Vec2f &uv2 = current_tri.uv2;

        TriangleGLSL tmp {};
        tmp.data1 = Vec4f(v0.x, v0.y, v0.z, (float) current_tri.mat_index);
        tmp.data2 = Vec4f(v1.x, v1.y, v1.z, 0.0f);
        tmp.data3 = Vec4f(v2.x, v2.y, v2.z, 0.0f);
        tmp.data4 = Vec4f(uv0.x, uv0.y, uv1.x, uv1.y);
        tmp.data5 = Vec4f(uv2.x, uv2.y, 0.0f, 0.0f);

        mesh_tris_ssbo.append(tmp);
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
