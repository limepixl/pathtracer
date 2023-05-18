#pragma once
#include "../math/mat4.hpp"
#include "../core/array.hpp"
#include "../defines.hpp"
#include "triangle.hpp"

struct Mesh
{
    Array<struct Triangle> triangles;
    Array<struct MaterialGLSL> materials;

    Mat4f model_matrix;
    uint32 texture_array {};

    Mesh() = default;

    Mesh(Array<struct Triangle> &triangles,
         Array<struct MaterialGLSL> &materials,
         Mat4f &model_matrix,
         uint32 texture_array);

    Array<TriangleGLSL> ConvertToSSBOFormat();

    void ApplyModelTransform(bool rotation = false);
};

