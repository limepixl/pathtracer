#include "material.hpp"
#include "../math/math.hpp"

MaterialGLSL::MaterialGLSL()
        : data1(0.0f), data2(0.0f), data3(0.0f, 0.0f, 0.0f, -1.0f) {}

MaterialGLSL::MaterialGLSL(const glm::vec3 &diffuse,
                           const glm::vec3 &specular,
                           const glm::vec3 &Le,
                           float roughness,
                           int diffuse_tex_index,
                           MaterialType type)
{
    data1 = glm::vec4(diffuse.x, diffuse.y, diffuse.z, roughness);
    data2 = glm::vec4(specular.x, specular.y, specular.z, (float) type);
    data3 = glm::vec4(Le.x, Le.y, Le.z, (float) diffuse_tex_index);

    if (type == MaterialType::MATERIAL_SPECULAR_METAL)
    {
        data1.w = pixl::max(0.01f, data1.w);
    }
}

glm::vec3 MaterialGLSL::diffuse() const
{
	return {data1.x, data1.y, data1.z};
}

glm::vec3 MaterialGLSL::specular() const
{
	return {data2.x, data2.y, data2.z};
}

glm::vec3 MaterialGLSL::emitted_radiance() const
{
	return {data3.x, data3.y, data3.z};
}
