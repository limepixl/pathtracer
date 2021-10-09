#include "intersect.hpp"
#include "math.hpp"
#include <math.h>

// Gets a point along the ray's direction vector.
// Assumes that the direction of the ray is normalized.
Vec3f PointAlongRay(Ray r, float32 t)
{
	return r.origin + r.direction * t;
}

Sphere CreateSphere(Vec3f origin, float32 radius, Material *mat)
{
	Sphere result = {origin, radius, mat};
	return result;
}

bool SphereIntersect(Ray ray, Sphere sphere, HitData *data, float32 &tmax)
{
	Vec3f oc = ray.origin - sphere.origin;
	float32 a = Dot(ray.direction, ray.direction);
	float32 b = 2.0f * Dot(oc, ray.direction);
	float32 c = Dot(oc, oc) - sphere.radius * sphere.radius;
	float32 discriminant = b*b - 4.0f*a*c;
	if(discriminant >= 0)
	{
		float32 sqrtDiscriminant = sqrtf(discriminant);
		if(discriminant == 0) // 2 equal real solutions
		{
			float32 t = -b / (2.0f * a);
			if(t > TMIN && t < tmax)
			{
				tmax = t;
				data->t = t;
				data->point = ray.origin + ray.direction * t;
				data->normal = data->point - sphere.origin;
				data->mat = sphere.mat;
				return true;
			}
		}
		else // 2 different real solutions
		{
			float32 t1 = (-b + sqrtDiscriminant) / (2.0f * a);
			float32 t2 = (-b - sqrtDiscriminant) / (2.0f * a);
			if(t1 > t2) 
			{
				float32 tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if(t1 > TMIN && t1 < tmax)
			{
				tmax = t1;
				data->t = t1;
				data->point = ray.origin + ray.direction * t1;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->mat = sphere.mat;
				return true;
			}
			else if(t2 > TMIN && t2 < tmax)
			{
				tmax = t2;
				data->t = t2;
				data->point = ray.origin + ray.direction * t2;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->mat = sphere.mat;
				return true;
			}
		}
	}

	return false;
}

Quad CreateQuad(Vec3f origin, Vec3f end, Vec3f normal, int8 component, Material *mat)
{
	Quad result = {origin, end, normal, component, mat};
	return result;
}

bool QuadIntersect(Ray ray, Quad quad, HitData *data, float32 &tmax)
{
	uint8 c = quad.component;
	float32 componentValue = quad.origin.values[c];

	// First we check if the ray hits the xy plane
	// ROz + t*RDz = Z
	float32 t = (componentValue - ray.origin.values[c]) / ray.direction.values[c];
	
	// t is either self intersecting, negative or too far away
	if(t < TMIN || t > tmax)
		return false;

	// Now we check if the point on that xy plane is within
	// the bounds of the quad we defined
	Vec3f pointOnPlane = ray.origin + ray.direction * t;

	// Not within the x interval of the quad
	if(c != 0 && (pointOnPlane.x < quad.origin.x || pointOnPlane.x > quad.end.x))
		return false;

	// Not within the y interval of the quad
	if(c != 1 && (pointOnPlane.y < quad.origin.y || pointOnPlane.y > quad.end.y))
		return false;

	// Not within the z interval of the quad
	if(c != 2 && (pointOnPlane.z < quad.origin.z || pointOnPlane.z > quad.end.z))
		return false;

	// Within quad!
	data->mat = quad.mat;
	data->normal = quad.normal;
	data->point = pointOnPlane;
	data->t = t;
	tmax = t;
	return true;
}

Box CreateBoxFromEndpoints(Vec3f origin, Vec3f end, Material *mat)
{
	// Each _min or _max quad has component equal
	// to its name. Ex.: xmin is a yz quad

	// xmin
	Vec3f v0 = origin;
	Vec3f v1 = CreateVec3f(origin.x, end.y, end.z);
	Vec3f n = CreateVec3f(-1.0f, 0.0f, 0.0f);
	Quad xmin = CreateQuad(v0, v1, n, 0, mat);

	// xmax
	v0.x = end.x;
	v1.x = end.x;
	n.x = 1.0f;
	Quad xmax = CreateQuad(v0, v1, n, 0, mat);

	// ymin
	v0 = origin;
	v1 = CreateVec3f(end.x, origin.y, end.z);
	n = CreateVec3f(0.0f, -1.0f, 0.0f);
	Quad ymin = CreateQuad(v0, v1, n, 1, mat);

	// ymax
	v0.y = end.y;
	v1.y = end.y;
	n.y = 1.0f;
	Quad ymax = CreateQuad(v0, v1, n, 1, mat);

	// zmin
	v0 = origin;
	v1 = CreateVec3f(end.x, end.y, origin.z);
	n = CreateVec3f(0.0f, 0.0f, -1.0f);
	Quad zmin = CreateQuad(v0, v1, n, 2, mat);

	// zmax
	v0.z = end.z;
	v1.z = end.z;
	n.z = 1.0f;
	Quad zmax = CreateQuad(v0, v1, n, 2, mat);

	return { origin, end, xmin, xmax, ymin, ymax, zmin, zmax };
}

// Just a convenience function
Box CreateBox(Vec3f origin, Vec3f dimensions, Material *mat)
{
	Vec3f end = origin + dimensions;
	return CreateBoxFromEndpoints(origin, end, mat);
}

bool BoxIntersect(Ray ray, Box box, HitData *data, float32 &tmax)
{
	bool hitAnyQuad = false;
	HitData resultData = {TMAX};

	for(uint8 i = 0; i < 6; i++)
	{
		Quad currentQuad = box.quads[i];
		HitData quadData = {};
		if(QuadIntersect(ray, currentQuad, &quadData, tmax))
		{
			if(quadData.t < resultData.t)
			{
				hitAnyQuad = true;
				resultData = quadData;
			}
		}
	}

	*data = resultData;
	return hitAnyQuad;
}

// NOTE: expects CCW winding order
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Material *mat)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	Vec3f normal = NormalizeVec3f(Cross(A, B));
	return {v0, v1, v2, normal, A, B, mat};
}

Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, Material *mat)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	return {v0, v1, v2, normal, A, B, mat};
}

// Moller–Trumbore ray-triangle intersection algorithm
bool TriangleIntersect(Ray ray, Triangle *tri, HitData *data, float32 &tmax)
{
#if 0
	Vec3f pvec = Cross(ray.direction, tri->edge2);
	float32 determinant = Dot(tri->edge1, pvec);

	// Ray direction parallel to the triangle plane
    if(determinant < EPSILON)
        return false;    

    float32 inv_determinant = 1.0f / determinant;
    Vec3f tvec = ray.origin - tri->v0;
    float32 u = Dot(tvec, pvec) * inv_determinant;
    if((u < 0.0f) || (u > 1.0f))
        return false;

    Vec3f qvec = Cross(tvec, tri->edge1);
    float32 v = inv_determinant * Dot(ray.direction, qvec);
    if((v < 0.0f) || (u + v > 1.0f))
        return false;

	// Computing t
    float32 t = inv_determinant * Dot(tri->edge2, qvec);
    if(t > TMIN && t < tmax)
    {
		tmax = t;
		data->t = t;
        data->point = ray.origin + ray.direction * t + EPSILON * tri->normal;
		data->normal = tri->normal;
		data->mat = tri->mat;
        return true;
    }
    
	return false;
#endif

	// compute plane's normal
    Vec3f v0v1 = tri->v1 - tri->v0; 
    Vec3f v0v2 = tri->v2 - tri->v0; 
    // no need to normalize
    Vec3f N = Cross(v0v1, v0v2); // N 
    float32 denom = Dot(N, N); 
 
    // Step 1: finding P
 
    // check if ray and plane are parallel ?
    float32 NdotRayDirection = Dot(N, ray.direction); 
    if (Abs(NdotRayDirection) < EPSILON) // almost 0 
        return false; // they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float32 d = Dot(N, tri->v0); 
 
    // compute t (equation 3)
    float32 t = (Dot(N, ray.origin) + d) / NdotRayDirection; 
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind 
 
    // compute the intersection point using equation 1
    Vec3f P = ray.origin + t * ray.direction; 
 
    // Step 2: inside-outside test
    Vec3f C; // vector perpendicular to triangle's plane 
 
    // edge 0
    Vec3f edge0 = tri->v1 - tri->v0; 
    Vec3f vp0 = P - tri->v0; 
    C = Cross(edge0, vp0); 
    if (Dot(N, C) < 0) return false; // P is on the right side 
 
    // edge 1
	float32 u, v;
    Vec3f edge1 = tri->v2 - tri->v1; 
    Vec3f vp1 = P - tri->v1; 
    C = Cross(edge1, vp1); 
    if ((u = Dot(N, C)) < 0)  return false; // P is on the right side 
 
    // edge 2
    Vec3f edge2 = tri->v0 - tri->v2; 
    Vec3f vp2 = P - tri->v2; 
    C = Cross(edge2, vp2); 
    if ((v = Dot(N, C)) < 0) return false; // P is on the right side; 
 
    u /= denom; 
    v /= denom; 

	data->t = t;
	data->normal = N;
	data->mat = tri->mat;
	data->point = ray.origin + ray.direction * t;
 
    return true; // this ray hits the triangle 
}

// TODO: replace with actual transformation matrices
void ApplyScaleToTriangle(Triangle *tri, Vec3f scaleVec)
{
	tri->v0.x *= scaleVec.x;
	tri->v1.x *= scaleVec.x;
	tri->v2.x *= scaleVec.x;

	tri->v0.y *= scaleVec.y;
	tri->v1.y *= scaleVec.y;
	tri->v2.y *= scaleVec.y;

	tri->v0.z *= scaleVec.z;
	tri->v1.z *= scaleVec.z;
	tri->v2.z *= scaleVec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}

// TODO: replace with actual transformation matrices
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translationVec)
{
	tri->v0.x += translationVec.x;
	tri->v1.x += translationVec.x;
	tri->v2.x += translationVec.x;

	tri->v0.y += translationVec.y;
	tri->v1.y += translationVec.y;
	tri->v2.y += translationVec.y;

	tri->v0.z += translationVec.z;
	tri->v1.z += translationVec.z;
	tri->v2.z += translationVec.z;

	tri->edge1 = tri->v1 - tri->v0;
	tri->edge2 = tri->v2 - tri->v0;
}

bool AABBIntersect(Ray ray, AABB aabb)
{
	// First check if ray origin is within AABB
	Vec3f &ro = ray.origin;
	if(ro >= aabb.bmin && ro <= aabb.bmax)
		return true;

	float32 t0x, t1x, t0y, t1y, t0z, t1z;
	Vec3f inverseDir = 1.0f / ray.direction;
	
	// X axis interval
	if(inverseDir.x >= 0)
	{
		t0x = (aabb.bmin.x - ray.origin.x) * inverseDir.x;
		t1x = (aabb.bmax.x - ray.origin.x) * inverseDir.x;
	}
	else
	{
		t0x = (aabb.bmax.x - ray.origin.x) * inverseDir.x;
		t1x = (aabb.bmin.x - ray.origin.x) * inverseDir.x;
	}
	
	// Y axis interval
	if(inverseDir.y >= 0)
	{
		t0y = (aabb.bmin.y - ray.origin.y) * inverseDir.y;
		t1y = (aabb.bmax.y - ray.origin.y) * inverseDir.y;
	}
	else
	{
		t0y = (aabb.bmax.y - ray.origin.y) * inverseDir.y;
		t1y = (aabb.bmin.y - ray.origin.y) * inverseDir.y;
	}

	if(t0x > t1y || t0y > t1x)
		return false;

	float32 tmin = Max(t0x, t0y);
	float32 tmax = Min(t1x, t1y);

	// Z axis interval
	if(inverseDir.z >= 0)
	{
		t0z = (aabb.bmin.z - ray.origin.z) * inverseDir.z;
		t1z = (aabb.bmax.z - ray.origin.z) * inverseDir.z;
	}
	else
	{
		t0z = (aabb.bmax.z - ray.origin.z) * inverseDir.z;
		t1z = (aabb.bmin.z - ray.origin.z) * inverseDir.z;
	}

	if(tmin > t1z || t0z > tmax)
		return false;

	tmin = Max(tmin, t0z);
	tmax = Min(tmax, t1z);

	if(tmin <= TMIN || tmin >= TMAX)
		return false;

	return true;
}

TriangleModel CreateTriangleModel(Triangle *tris, int32 numTris, Mat4f modelMatrix)
{
	Vec3f maxVec = CreateVec3f(INFINITY);
	Vec3f minVec = CreateVec3f(-INFINITY);
	AABB aabb = {maxVec, minVec};

	for(int32 tIndex = 0; tIndex < numTris; tIndex++)
	{
		Vec3f &v0 = tris[tIndex].v0;
		Vec3f &v1 = tris[tIndex].v1;
		Vec3f &v2 = tris[tIndex].v2;

		// Apply model's transformation matrix to vertices
		v0 = modelMatrix * v0;
		v1 = modelMatrix * v1;
		v2 = modelMatrix * v2;
		tris[tIndex].edge1 = v1 - v0;
		tris[tIndex].edge2 = v2 - v0;

		aabb.bmin = MinComponentWise(aabb.bmin, v0);
		aabb.bmin = MinComponentWise(aabb.bmin, v1);
		aabb.bmin = MinComponentWise(aabb.bmin, v2);

		aabb.bmax = MaxComponentWise(aabb.bmax, v0);
		aabb.bmax = MaxComponentWise(aabb.bmax, v1);
		aabb.bmax = MaxComponentWise(aabb.bmax, v2);
	}

	return {tris, numTris, aabb, modelMatrix};
}

bool TriangleModelIntersect(Ray ray, TriangleModel triModel, HitData *data, float32 &tmax)
{
	// Check if ray intersects AABB at all before checking
	// intersections with any of the model's triangles
	bool intersectsAABB = AABBIntersect(ray, triModel.aabb);
	if(intersectsAABB)
	{
		float32 localtmax = tmax;

		bool hitAnyTri = false;
		HitData resultData = {localtmax};
		
		for(int32 tri = 0; tri < triModel.numTrianges; tri++)
		{
			HitData currentData = {};
			Triangle *current = &(triModel.triangles[tri]);
			bool intersectTri = TriangleIntersect(ray, current, &currentData, localtmax);
			if(intersectTri)
			{
				hitAnyTri = true;
				resultData = currentData;

				resultData.objectIndex = tri;
				resultData.objectType = ObjectType::TRIANGLE;
			}
		}

		if(hitAnyTri)
		{
			*data = resultData;
			tmax = localtmax;
		}
	
		return hitAnyTri;
	}

	return false;
}

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return {obj, type};
}

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Quad *quads, int32 numQuads,
					 TriangleModel *triModels, int32 numTriModels,
					 uint32 *lightTris, int32 numLightTris)
{
	return {spheres, numSpheres, 
			quads, numQuads, 
			triModels, numTriModels,
			lightTris, numLightTris};
}

// TODO: clean this up
bool Intersect(Ray ray, Scene scene, HitData *data)
{
	bool hitAnything = false;
	HitData resultData = {TMAX};

	float32 tmax = TMAX;

	for(int32 i = 0; i < scene.numSpheres; i++)
	{
		HitData currentData = {};
		Sphere current = scene.spheres[i];
		bool intersect = SphereIntersect(ray, current, &currentData, tmax);
		if(intersect)
		{
			// Found closer hit, store it.
			hitAnything = true;
			resultData = currentData;
			
			resultData.objectIndex = i;
			resultData.objectType = ObjectType::SPHERE;
		}
	}

	for(int32 i = 0; i < scene.numQuads; i++)
	{
		HitData currentData = {};
		Quad current = scene.quads[i];
		bool intersect = QuadIntersect(ray, current, &currentData, tmax);
		if(intersect)
		{
			// Found closer hit, store it.
			hitAnything = true;
			resultData = currentData;
						
			resultData.objectIndex = i;
			resultData.objectType = ObjectType::QUAD;
		}
	}

	for(int32 i = 0; i < scene.numTriModels; i++)
	{
		HitData currentData = {};
		TriangleModel current = scene.triModels[i];
		bool intersect = TriangleModelIntersect(ray, current, &currentData, tmax);
		if(intersect)
		{
			// Found closer hit, store it.
			hitAnything = true;
			resultData = currentData;
		}
	}

	*data = resultData;
	return hitAnything;
}

// Area functions
float32 Area(Quad *quad)
{
	Vec3f v0 = quad->origin;
	Vec3f v2 = quad->end;
	Vec3f dims = v2 - v0;

	float32 quadArea = 0.0f;
	if(quad->component == 0) // yz
		quadArea = dims.y * dims.z;
	else if(quad->component == 1) // xz
		quadArea = dims.x * dims.z;
	else // xy
		quadArea = dims.x * dims.y;

	return quadArea;
}

float32 Area(Sphere *sphere)
{
	return 4.0f * PI * sphere->radius * sphere->radius;
};

float32 Area(Triangle *tri)
{
	return sqrtf(Dot(tri->edge1, tri->edge1) * Dot(tri->edge2, tri->edge2)) / 2.0f;
}