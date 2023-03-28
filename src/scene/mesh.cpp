#include "mesh.h"
#include "triangle.hpp"
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
