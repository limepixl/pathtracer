#pragma once
#include "vec.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

Vec3f PointAlongRay(Ray r, float32 t);

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

Sphere CreateSphere(Vec3f origin, float32 radius, int16 materialIndex);
bool SphereIntersect(Ray ray, Sphere sphere, HitData *data);

struct Quad
{
	Vec3f origin;
	Vec3f end;
	Vec3f normal;
	int8 component; // 0 for x, 1 for y, 2 for z
	
	int16 materialIndex;
};

Quad CreateQuad(Vec3f origin, Vec3f end, Vec3f normal, int8 component, int16 materialIndex);
bool QuadIntersect(Ray ray, Quad quad, HitData *data);

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

Box CreateBoxFromEndpoints(Vec3f origin, Vec3f end, int16 materialIndex);
Box CreateBox(Vec3f origin, Vec3f dimensions, int16 materialIndex);
bool BoxIntersect(Ray ray, Box box, HitData *data);

struct Triangle
{
	Vec3f v0, v1, v2;
	Vec3f normal;

	// Precomputed values
	Vec3f edge1, edge2;
};

Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal);
bool TriangleIntersect(Ray ray, Triangle *tri, HitData *data);
void ApplyScaleToTriangle(Triangle *tri, Vec3f scaleVec);
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translationVec);

struct AABB
{
	Vec3f bmin;
	Vec3f bmax;
};

bool AABBIntersect(Ray ray, AABB aabb);

struct TriangleModel
{
	Triangle *triangles;
	int32 numTrianges;
	AABB aabb;

	int16 materialIndex;

	// TODO: store a model matrix that can be applied to vertices
};

TriangleModel CreateTriangleModel(Triangle *tris, int32 numTris, int16 materialIndex);
bool TriangleModelIntersect(Ray ray, TriangleModel triModel, HitData *data);

enum LightSourceType
{
	QUAD
};

struct LightSource
{
	void *obj;
	LightSourceType type;
};

LightSource CreateLightSource(void *obj, LightSourceType type);

struct Scene
{
	Sphere *spheres;
	int32 numSpheres;

	Quad *quads;
	int32 numQuads;

	TriangleModel *triModels;
	int32 numTriModels;

	LightSource *lightSources;
	int32 numLightSources;

	struct Material *materials;
	int32 numMaterials;
};

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Quad *quads, int32 numQuads,
					 TriangleModel *triModels, int32 numTriModels,
					 LightSource *lights, int32 numLights,
					 struct Material *materials, int32 numMaterials);

bool Intersect(Ray ray, Scene scene, HitData *data);