#pragma once
#include "mat4.hpp"
#include "bvh.hpp"

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

Vec3f PointAlongRay(Ray r, float32 t);

enum ObjectType
{
	SPHERE = 0,
	QUAD,
	TRIANGLE
};

struct HitData
{
	float32 t;
	Vec3f normal;
	Vec3f point;

	struct Material *mat;

	int32 objectIndex;
	ObjectType objectType;
};

struct Sphere
{
	Vec3f origin;
	float32 radius;
	
	struct Material *mat;
};

Sphere CreateSphere(Vec3f origin, float32 radius, struct Material *mat);
bool SphereIntersect(Ray ray, Sphere sphere, HitData *data, float32 &tmax);

struct Quad
{
	Vec3f origin;
	Vec3f end;
	Vec3f normal;
	int8 component; // 0 for x, 1 for y, 2 for z
	
	struct Material *mat;
};

Quad CreateQuad(Vec3f origin, Vec3f end, Vec3f normal, int8 component, struct Material *mat);
bool QuadIntersect(Ray ray, Quad quad, HitData *data, float32 &tmax);

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

Box CreateBoxFromEndpoints(Vec3f origin, Vec3f end, struct Material *mat);
Box CreateBox(Vec3f origin, Vec3f dimensions, struct Material *mat);
bool BoxIntersect(Ray ray, Box box, HitData *data, float32 &tmax);

struct Triangle
{
	Vec3f v0, v1, v2;
	Vec3f normal;

	// Precomputed values
	Vec3f edge1, edge2;

	struct Material *mat;
};

Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, struct Material *mat);
Triangle CreateTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f normal, struct Material *mat);
bool TriangleIntersect(Ray ray, Triangle *tri, HitData *data, float32 &tmax);
void ApplyScaleToTriangle(Triangle *tri, Vec3f scaleVec);
void ApplyTranslationToTriangle(Triangle *tri, Vec3f translationVec);

bool AABBIntersect(Ray ray, AABB aabb);

struct TriangleModel
{
	Triangle *triangles;
	int32 numTrianges;
	
	AABB aabb;

	Mat4f modelMatrix;;
};

TriangleModel CreateTriangleModel(Triangle *tris, int32 numTris, Mat4f modelMatrix);
bool TriangleModelIntersect(Ray ray, TriangleModel triModel, HitData *data, float32 &tmax);

enum class LightSourceType
{
	QUAD,
	SPHERE,
	TRIANGLE
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

	uint32 *lightTris;
	int32 numLightTris;
};

Scene ConstructScene(Sphere *spheres, int32 numSpheres, 
					 Quad *quads, int32 numQuads,
					 TriangleModel *triModels, int32 numTriModels,
					 uint32 *lightTris, int32 numLightTris);

bool Intersect(Ray ray, Scene scene, HitData *data);

// Area functions
float32 Area(Quad *quad);
float32 Area(Sphere *sphere);
float32 Area(Triangle *tri);