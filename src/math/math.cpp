#include "math.hpp"
#include "../scene/triangle.hpp"

/*
	Functions
*/

int16 pixl::clamp(int16 value, int16 max)
{
	if (value > max)
	{
		return max;
	}

	return value;
}

float pixl::max(float a, float b)
{
	return a > b ? a : b;
}

double pixl::max(double a, double b)
{
	return a > b ? a : b;
}

int32 pixl::max(int32 a, int32 b)
{
	return a > b ? a : b;
}

int16 pixl::max(int16 a, int16 b)
{
	return a > b ? a : b;
}

uint32 pixl::max(uint32 a, uint32 b)
{
	return a > b ? a : b;
}

float pixl::min(float a, float b)
{
	return a < b ? a : b;
}

double pixl::min(double a, double b)
{
	return a < b ? a : b;
}

uint32 pixl::min(uint32 a, uint32 b)
{
	return a < b ? a : b;
}

uint16 pixl::min(uint16 a, uint16 b)
{
	return a < b ? a : b;
}

float pixl::step(float edge, float x)
{
	return x < edge ? 0.0f : 1.0f;
}

float pixl::ceil(float num)
{
	if (num - (float)((int32)num) > 0.0f)
	{
		return float((int32)num + 1);
	}

	return num;
}

void pixl::swap(float *v1, float *v2)
{
	float tmp = *v1;
	*v1 = *v2;
	*v2 = tmp;
}

// PCG variants of the above functions
float pixl::random_number_normalized_PCG(pcg32_random_t *rngptr)
{
	double d = ldexp(pcg32_random_r(rngptr), -32);
	return (float)d;
}

glm::vec2 pixl::random_vec2_PCG(pcg32_random_t *rngptr)
{
	float r1 = random_number_normalized_PCG(rngptr);
	while (r1 < 0.0001f || r1 > 0.9999f)
	{
		r1 = random_number_normalized_PCG(rngptr);
	}

	float r2 = random_number_normalized_PCG(rngptr);
	while (r2 < 0.0001f || r2 > 0.9999f)
	{
		r2 = random_number_normalized_PCG(rngptr);
	}

	return { r1, r2 };
}

glm::vec3 pixl::random_vec3_PCG(pcg32_random_t *rngptr)
{
	float r1 = random_number_normalized_PCG(rngptr);
	while (r1 < 0.0001f || r1 > 0.9999f)
	{
		r1 = random_number_normalized_PCG(rngptr);
	}

	float r2 = random_number_normalized_PCG(rngptr);
	while (r2 < 0.0001f || r2 > 0.9999f)
	{
		r2 = random_number_normalized_PCG(rngptr);
	}

	float r3 = random_number_normalized_PCG(rngptr);
	while (r1 < 0.0001f || r1 > 0.9999f)
	{
		r1 = random_number_normalized_PCG(rngptr);
	}

	return { r1, r2, r3 };
}

glm::vec3 pixl::map_to_unit_sphere(glm::vec2 vec2)
{
	// First we map [0,1] to [0,2] and subtract one to map
	// that to [-1, 1], which is the range of cosine.
	float cosTheta = 2.0f * vec2.x - 1.0f;

	// We can directly map phi to [0, 2PI] from [0, 1] by just
	// multiplying it with 2PI
	float Phi = 2.0f * PI * vec2.y;

	// sin^2(x) = 1 - cos^2(x)
	// sin(x) = sqrt(1 - cos^2(x))
	float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

	float sinPhi = sinf(Phi);
	float cosPhi = cosf(Phi);

	// Just a conversion between spherical and Cartesian coordinates
	return { sinTheta * cosPhi, cosTheta, sinTheta * sinPhi };
}

glm::vec3 pixl::map_to_unit_hemisphere_cosine_weighted_criver(glm::vec2 uv, glm::vec3 normal)
{
	glm::vec3 p = map_to_unit_sphere(uv);
	return normalize(p + normal);
}

glm::vec3 pixl::map_to_triangle(glm::vec2 vec2, Triangle tri)
{
	float u = vec2.x;
	float v = vec2.y;

	if (u + v > 1.0f)
	{
		// The generated point is outside triangle but
		// within the parallelogram defined by the 2 edges
		// of the triangle (v1-v0 and v2-v0)
		u = 1.0f - u;
		v = 1.0f - v;
	}

	glm::vec3 p = u * tri.edge1 + v * tri.edge2;
	return p + tri.v0;
}

// https://jcgt.org/published/0006/01/01/
void pixl::orthonormal_basis(glm::vec3 &n, glm::vec3 &t, glm::vec3 &bt)
{
	float sign = copysignf(1.0f, n.z);
	float a = -1.0f / (sign + n.z);
	float b = n.x * n.y * a;

	t = glm::vec3(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
	bt = glm::vec3(b, sign + n.y * n.y * a, -n.y);
}

glm::mat3 pixl::construct_TNB_matrix(glm::vec3 &n)
{
	glm::vec3 t {}, bt {};
	orthonormal_basis(n, t, bt);
	glm::mat3 res(t, n, bt);
	return glm::transpose(res);
}

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
glm::vec2 pixl::octahedral_wrap(const glm::vec2 &v)
{
	glm::vec2 v_xy(
		v.x >= 0.0f ? 1.0f : -1.0f,
		v.y >= 0.0f ? 1.0f : -1.0f
	);
	return (1.0f - abs(glm::vec2(v.y, v.x))) * v_xy;
}
glm::vec2 pixl::octahedral_normal_encoding(glm::vec3 n)
{
	n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));
	glm::vec2 n_xy(n.x, n.y);
	n_xy = (n.z >= 0.0f) ? n_xy : octahedral_wrap(n_xy);
	return n_xy;
}
