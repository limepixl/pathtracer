#include "sphere.hpp"
#include "../math/math.hpp"

float Area(Sphere *sphere)
{
	return 4.0f * PI * sphere->radius * sphere->radius;
};

Sphere CreateSphere(Vec3f origin, float radius, uint32 mat_index)
{
	Sphere result = { origin, radius, mat_index };
	return result;
}