#pragma once
#include "../defines.hpp"
#include <glm/vec3.hpp>

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 inv_dir;
};
