#include "material.hpp"
#include "../math/math.hpp"

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

Vec3f MaterialGLSL::diffuse() const
{
	return {data1.x, data1.y, data1.z};
}

Vec3f MaterialGLSL::specular() const
{
	return {data2.x, data2.y, data2.z};
}

Vec3f MaterialGLSL::emitted_radiance() const
{
	return {data3.x, data3.y, data3.z};
}
