#pragma once
#include "../math/vec.hpp"

enum MaterialType
{
  MATERIAL_LAMBERTIAN = 0,
  MATERIAL_OREN_NAYAR,
  MATERIAL_SPECULAR_METAL
};

struct Material
{
  MaterialType type;
  Vec3f diffuse;
  Vec3f specular;
  float n_spec;

  Vec3f Le; // emmision of light

  // Optional
  char name[32];
};

Material CreateMaterial(MaterialType type,
                        Vec3f diffuse,
                        Vec3f specular,
                        float n_spec = 0.0f,
                        Vec3f Le = Vec3f(0.0f),
                        const char *name = "");

struct MaterialGLSL
{
  Vec4f data1; // diff.x, diff.y, diff.z, roughness
  Vec4f data2; // spec.x, spec.y, spec.z, n_spec
  Vec4f data3; // Le.x, Le.y, Le.z, mat_type
  Vec4f data4; // diffuse_tex_indx, 0, 0, 0

  MaterialGLSL();

  MaterialGLSL(const Vec3f &diffuse,
               const Vec3f &specular,
               const Vec3f &Le,
               float roughness,
               float specular_exponent,
               int diffuse_tex_index,
               MaterialType type);
};
