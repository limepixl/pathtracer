#pragma once

struct SphereGLSL
{
	glm::vec4 data {}; // o.x, o.y, o.z, radius
    uint32 mat_index[4] {0}; // mat_index, 0, 0, 0

    SphereGLSL() = default;

    SphereGLSL(glm::vec3 origin, float radius, uint32 mat_index)
        : data(glm::vec4(origin.x, origin.y, origin.z, radius))
    {
        this->mat_index[0] = mat_index;
    }
};
