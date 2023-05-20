#pragma once
#include "../../thirdparty/pcg-c-basic-0.9/pcg_basic.h"
#include "../defines.hpp"
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Triangle;

constexpr float PI = 3.14159265f;
constexpr float EPSILON = 0.0001f;

/*
	Functions
*/

namespace pixl
{
    float sign(float value);

    glm::vec3 sign(glm::vec3 value);

    float abs(float value);

    glm::vec3 abs(glm::vec3 value);

    float radians(float degrees);

    int16 clamp(int16 value, int16 max);

    float dot(glm::vec3 vec1, glm::vec3 vec2);

	float dot(glm::vec4 vec1, glm::vec4 vec2);

    glm::vec3 cross(const glm::vec3 &a, const glm::vec3 &b);

    float max(float a, float b);

    double max(double a, double b);

    int32 max(int32 a, int32 b);

    int16 max(int16 a, int16 b);

    uint32 max(uint32 a, uint32 b);

    glm::vec3 max_component_wise(glm::vec3 &a, glm::vec3 &b);

    float min(float a, float b);

    double min(double a, double b);

    uint32 min(uint32 a, uint32 b);

    uint16 min(uint16 a, uint16 b);

    glm::vec3 min_component_wise(glm::vec3 &a, glm::vec3 &b);

    float step(float edge, float x);

    glm::vec3 step(glm::vec3 edge, glm::vec3 x);

    float ceil(float num);

    void swap(float *v1, float *v2);

    glm::vec3 normalize(glm::vec3 vec);

	glm::vec4 normalize(glm::vec4 vec);

    float random_number_normalized_PCG(pcg32_random_t *rngptr);

    glm::vec2 random_vec2_PCG(pcg32_random_t *rngptr);

    glm::vec3 random_vec3_PCG(pcg32_random_t *rngptr);

    glm::vec3 map_to_unit_sphere(glm::vec2 vec2);

    glm::vec3 map_to_unit_hemisphere_cosine_weighted_criver(glm::vec2 uv, glm::vec3 normal);

	glm::vec3 map_to_triangle(glm::vec2 vec2, Triangle tri);

    glm::vec3 reflect(glm::vec3 dir, glm::vec3 normal);

    void orthonormal_basis(glm::vec3 &n, glm::vec3 &t, glm::vec3 &bt);

	glm::mat3 construct_TNB_matrix(glm::vec3 &n);
	}

/*
	Operators that use the utility functions above
*/

bool operator==(const glm::vec3 &lhs, const glm::vec3 &rhs);

bool operator!=(const glm::vec3 &lhs, const glm::vec3 &rhs);
