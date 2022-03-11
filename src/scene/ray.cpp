#include "ray.hpp"

// Gets a point along the ray's direction vector.
// Assumes that the direction of the ray is normalized.
Vec3f PointAlongRay(Ray r, float t)
{
	return r.origin + r.direction * t;
}