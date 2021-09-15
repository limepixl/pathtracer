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
	int8 component; // 0 for x, 1 for y, 2 for z
	
	int16 materialIndex;
};

Quad CreateQuad(Vec3f origin, Vec3f end, int8 component, int16 materialIndex)
{
	Quad result = {origin, end, component, materialIndex};
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
	// Check which way the normal points to
	float32 normalSign = Sign(ray.origin.values[c] - pointOnPlane.values[c]);

	// If this is true then the ray origin component is
	// right inside the plane, which shouldn't happen if
	// we avoid self intersection!
	if(normalSign == 0.0f)
	{
		printf("help!\n");
		normalSign = 1.0f;
	}

	data->materialIndex = quad.materialIndex;
	data->normal = {0.0f, 0.0f, 0.0f};
	data->normal.values[c] = normalSign;
	data->point = pointOnPlane;
	data->t = t;
	return true;
}

struct Scene
{
	Sphere *spheres;
	int32 numSpheres;

	Quad *quads;
	int32 numQuads;

	struct Material *materials;
	int32 numMaterials;
};

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Quad *quads, int32 numQuads,
					 struct Material *materials, int32 numMaterials)
{
	return {spheres, numSpheres, quads, numQuads, materials, numMaterials};
}

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

	*data = resultData;
	return hitAnything;
}