#pragma once
#include "../../thirdparty/pcg-c-basic-0.9/pcg_basic.h"
#include "../defines.hpp"
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Triangle;

constexpr float PI = 3.14159265f;
constexpr float EPSILON = 0.0001f;

/*
	Functions
*/

namespace pixl
{
int16 clamp(int16 value, int16 max);

float max(float a, float b);

double max(double a, double b);

int32 max(int32 a, int32 b);

int16 max(int16 a, int16 b);

uint32 max(uint32 a, uint32 b);

float min(float a, float b);

double min(double a, double b);

uint32 min(uint32 a, uint32 b);

uint16 min(uint16 a, uint16 b);

float step(float edge, float x);

float ceil(float num);

void swap(float *v1, float *v2);

float random_number_normalized_PCG(pcg32_random_t *rngptr);

glm::vec2 random_vec2_PCG(pcg32_random_t *rngptr);

glm::vec3 random_vec3_PCG(pcg32_random_t *rngptr);

glm::vec3 map_to_unit_sphere(glm::vec2 vec2);

glm::vec3 map_to_unit_hemisphere_cosine_weighted_criver(glm::vec2 uv, glm::vec3 normal);

glm::vec3 map_to_triangle(glm::vec2 vec2, Triangle tri);

void orthonormal_basis(glm::vec3 &n, glm::vec3 &t, glm::vec3 &bt);

glm::mat3 construct_TNB_matrix(glm::vec3 &n);
}
