#include "sphere.hpp"
#include "material.hpp"
#include "../math/math.hpp"

SphereGLSL::SphereGLSL(glm::vec3 origin, float radius, uint32 mat_index)
	: data(glm::vec4(origin.x, origin.y, origin.z, radius))
{
	this->mat_index.x = mat_index;
}

Array<uint32> FindEmissiveSpheres(Array<SphereGLSL> &spheres, Array<MaterialGLSL> &materials)
{
	Array<uint32> emissive_spheres;
	for (uint32 i = 0; i < spheres.size; i++)
	{
		MaterialGLSL &mat = materials[spheres[i].mat_index.x];
		if (glm::all(glm::greaterThan(mat.emitted_radiance(), glm::vec3(EPSILON))))
		{
			emissive_spheres.append(i);
		}
	}

	return emissive_spheres;
}
