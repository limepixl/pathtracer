#include "material.hpp"
#include <cstring>
#include "../math/math.hpp"

Material CreateMaterial(MaterialType type, Vec3f diffuse, Vec3f specular, float n_spec, Vec3f Le, const char *name)
{
    // NOTE: currently not importing models with O-N BRDF so I divide
    // by PI in the shader.
    if (type == MaterialType::MATERIAL_LAMBERTIAN)
    {
        diffuse /= PI;
    }

    Material result = {type, diffuse, specular, n_spec, Le, {}};

    size_t name_length = strlen(name);
    if (name_length > 0)
    {
        strncpy(result.name, name, 31);

        // Truncate string
        name_length = name_length <= 31 ? name_length : 31;
        result.name[name_length] = '\0';
    }

    return result;
}

MaterialGLSL::MaterialGLSL()
        : data1(0.0f), data2(0.0f), data3(0.0f), data4(-1.0f) {}

MaterialGLSL::MaterialGLSL(const Vec3f &diffuse,
                           const Vec3f &specular,
                           const Vec3f &Le,
                           float roughness,
                           float specular_exponent,
                           int diffuse_tex_index,
                           MaterialType type)
{
    data1 = Vec4f(diffuse.x, diffuse.y, diffuse.z, roughness);
    data2 = Vec4f(specular.x, specular.y, specular.z, specular_exponent);
    data3 = Vec4f(Le.x, Le.y, Le.z, (float) type);
    data4 = Vec4f((float) diffuse_tex_index, 0.0f, 0.0f, 0.0f);

    if (type == MaterialType::MATERIAL_SPECULAR_METAL)
    {
        data1.w = pixl::max(0.01f, data1.w);
    }
}


