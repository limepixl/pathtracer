#pragma once

struct HitData
{
	float32 t;
	Vec3f normal;
	Vec3f point;

	int16 materialIndex;
};

struct Sphere
{
	Vec3f origin;
	float32 radius;
	
	int16 materialIndex;
};

Sphere CreateSphere(Vec3f origin, float32 radius, int16 materialIndex)
{
	Sphere result = {origin, radius, materialIndex};
	return result;
}

bool SphereIntersect(Ray ray, Sphere sphere, HitData *data)
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
			if(t > TMIN && t < TMAX)
			{
				data->t = t;
				data->point = ray.origin + ray.direction * t;
				data->normal = data->point - sphere.origin;
				data->materialIndex = sphere.materialIndex;
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

			if(t1 > TMIN && t1 < TMAX)
			{
				data->t = t1;
				data->point = ray.origin + ray.direction * t1;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->materialIndex = sphere.materialIndex;
				return true;
			}
			else if(t2 > TMIN && t2 < TMAX)
			{
				data->t = t2;
				data->point = ray.origin + ray.direction * t2;
				data->normal = (data->point - sphere.origin) / sphere.radius;
				data->materialIndex = sphere.materialIndex;
				return true;
			}
		}
	}

	return false;
}

struct Quad
{
	Vec3f origin;
	Vec3f end;
	Vec3f normal;
	int8 component; // 0 for x, 1 for y, 2 for z
	
	int16 materialIndex;
};

Quad CreateQuad(Vec3f origin, Vec3f end, Vec3f normal, int8 component, int16 materialIndex)
{
	Quad result = {origin, end, normal, component, materialIndex};
	return result;
}

bool QuadIntersect(Ray ray, Quad quad, HitData *data)
{
	int8 c = quad.component;
	float32 componentValue = quad.origin.values[c];

	// First we check if the ray hits the xy plane
	// ROz + t*RDz = Z
	float32 t = (componentValue - ray.origin.values[c]) / ray.direction.values[c];
	
	// t is either self intersecting, negative or too far away
	if(t < TMIN || t > TMAX)
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
	data->materialIndex = quad.materialIndex;
	data->normal = quad.normal;
	data->point = pointOnPlane;
	data->t = t;
	return true;
}

struct Box
{
	Vec3f origin;
	Vec3f end;

	union
	{
		struct
		{
			Quad xmin, xmax, ymin, ymax, zmin, zmax;
		};
		Quad quads[6];
	};
};

Box CreateBoxFromEndpoints(Vec3f origin, Vec3f end, int16 materialIndex)
{
	// Each _min or _max quad has component equal
	// to its name. Ex.: xmin is a yz quad

	// xmin
	Vec3f v0 = origin;
	Vec3f v1 = CreateVec3f(origin.x, end.y, end.z);
	Vec3f n = CreateVec3f(-1.0f, 0.0f, 0.0f);
	Quad xmin = CreateQuad(v0, v1, n, 0, materialIndex);

	// xmax
	v0.x = end.x;
	v1.x = end.x;
	n.x = 1.0f;
	Quad xmax = CreateQuad(v0, v1, n, 0, materialIndex);

	// ymin
	v0 = origin;
	v1 = CreateVec3f(end.x, origin.y, end.z);
	n = CreateVec3f(0.0f, -1.0f, 0.0f);
	Quad ymin = CreateQuad(v0, v1, n, 1, materialIndex);

	// ymax
	v0.y = end.y;
	v1.y = end.y;
	n.y = 1.0f;
	Quad ymax = CreateQuad(v0, v1, n, 1, materialIndex);

	// zmin
	v0 = origin;
	v1 = CreateVec3f(end.x, end.y, origin.z);
	n = CreateVec3f(0.0f, 0.0f, -1.0f);
	Quad zmin = CreateQuad(v0, v1, n, 2, materialIndex);

	// zmax
	v0.z = end.z;
	v1.z = end.z;
	n.z = 1.0f;
	Quad zmax = CreateQuad(v0, v1, n, 2, materialIndex);

	return { origin, end, xmin, xmax, ymin, ymax, zmin, zmax };
}

// Just a convenience function
Box CreateBox(Vec3f origin, Vec3f dimensions, int16 materialIndex)
{
	Vec3f end = origin + dimensions;
	return CreateBoxFromEndpoints(origin, end, materialIndex);
}

bool BoxIntersect(Ray ray, Box box, HitData *data)
{
	bool hitAnyQuad = false;
	HitData resultData = {TMAX};

	for(int8 i = 0; i < 6; i++)
	{
		Quad currentQuad = box.quads[i];
		HitData quadData = {};
		if(QuadIntersect(ray, currentQuad, &quadData))
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

struct Triangle
{
	Vec3f v0, v1, v2;
	Vec3f normal;

	int16 materialIndex;
};

// NOTE: expects CCW winding order
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, int16 materialIndex)
{
	Vec3f A = v1 - v0;
	Vec3f B = v2 - v0;
	Vec3f normal = NormalizeVec3f(Cross(A, B));
	return {v0, v1, v2, normal, materialIndex};
}

bool TriangleIntersect(Ray ray, Triangle tri, HitData *data)
{
	// First we check intersection with plane
	// Plane eq: Ax+By+Cz+D=0

	// Check if ray is parallel to triangle plane
	float32 NdotRD = Dot(tri.normal, ray.direction);
	if(Abs(NdotRD) <= FLOAT_EQUALITY_PRECISION)
		return false;

	// Distance of triangle plane to origin (0, 0, 0)
	float32 D = Dot(tri.normal, tri.v0);

	// Find point on triangle plane
	if(NdotRD < 0.0f)
		NdotRD *= -1.0f;
	float32 t = -(D + Dot(tri.normal, ray.origin)) / NdotRD;
	if(t <= TMIN || t >= TMAX)
		return false;

	Vec3f pointOnTrianglePlane = ray.origin + t * ray.direction;

	Vec3f edge0 = tri.v1 - tri.v0;
	Vec3f v0P = pointOnTrianglePlane - tri.v0;

	Vec3f edge1 = tri.v2 - tri.v1;
	Vec3f v1P = pointOnTrianglePlane - tri.v1;
	
	Vec3f edge2 = tri.v0 - tri.v2;
	Vec3f v2P = pointOnTrianglePlane - tri.v2;
	
	Vec3f C = Cross(edge0, v0P);
	if(Dot(tri.normal, C) < 0.0f) return false;

	C = Cross(edge1, v1P);
	if(Dot(tri.normal, C) < 0.0f) return false;

	C = Cross(edge2, v2P);
	if(Dot(tri.normal, C) < 0.0f) return false;

	data->t = t;
	data->normal = tri.normal;
	data->point = pointOnTrianglePlane;
	data->materialIndex = tri.materialIndex;
	return true;
}

enum LightSourceType
{
	QUAD
};

struct LightSource
{
	void *obj;
	LightSourceType type;
};

LightSource CreateLightSource(void *obj, LightSourceType type)
{
	return {obj, type};
}

struct Scene
{
	Sphere *spheres;
	int32 numSpheres;

	Quad *quads;
	int32 numQuads;

	Triangle *triangles;
	int32 numTriangles;

	LightSource *lightSources;
	int32 numLightSources;

	struct Material *materials;
	int32 numMaterials;
};

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Quad *quads, int32 numQuads,
					 Triangle *triangles, int32 numTriangles,
					 LightSource *lights, int32 numLights,
					 struct Material *materials, int32 numMaterials)
{
	return {spheres, numSpheres, 
			quads, numQuads, 
			triangles, numTriangles,
			lights, numLights, 
			materials, numMaterials};
}

// TODO: clean this up
bool Intersect(Ray ray, Scene scene, HitData *data)
{
	bool hitAnything = false;
	HitData resultData = {TMAX};

	for(int32 i = 0; i < scene.numSpheres; i++)
	{
		HitData currentData = {};
		Sphere current = scene.spheres[i];
		bool intersect = SphereIntersect(ray, current, &currentData);
		if(intersect)
		{
			if(currentData.t < resultData.t)
			{
				// Found closer hit, store it.
				hitAnything = true;
				resultData = currentData;
			}
		}
	}

	for(int32 i = 0; i < scene.numQuads; i++)
	{
		HitData currentData = {};
		Quad current = scene.quads[i];
		bool intersect = QuadIntersect(ray, current, &currentData);
		if(intersect)
		{
			if(currentData.t < resultData.t)
			{
				// Found closer hit, store it.
				hitAnything = true;
				resultData = currentData;
			}
		}
	}

	for(int32 i = 0; i < scene.numTriangles; i++)
	{
		HitData currentData = {};
		Triangle current = scene.triangles[i];
		bool intersect = TriangleIntersect(ray, current, &currentData);
		if(intersect)
		{
			if(currentData.t < resultData.t)
			{
				// Found closer hit, store it.
				hitAnything = true;
				resultData = currentData;
			}
		}
	}

	*data = resultData;
	return hitAnything;
}