#include "loader.hpp"
#include "intersect.hpp"
#include "defines.hpp"
#include "math.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../tinyobjloader/tiny_obj_loader.h"
#include <vector>
#include <string>

bool LoadModelFromObj(const char *path, Triangle **outTris, int32 *numOutTris)
{
	printf("Loading .obj model from path: %s\n", path);

	tinyobj::attrib_t attrib = {};
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, path);

	// Print out any warnings or errors that tinyobjloader returned
	if(!warning.empty())
		printf("WARNING: %s\n", warning.c_str());
	
	if(!error.empty())
		printf("ERROR: %s\n", error.c_str());

	if(!res)
	{
		printf("Failed to load obj file at path: %s\n", path);
		return false;
	}

	int64 numVertices = (int64)(attrib.vertices.size() / 3);
	int64 numNormals = (int64)(attrib.normals.size() / 3);
	int64 numUVs = (int64)(attrib.texcoords.size() / 2);
	int32 numMaterials = (int32)(materials.size());
	int32 numShapes = (int32)(shapes.size());

	// TODO: material handling

	// TODO: compute AABB while loading

	std::vector<Triangle> tris;
	for(int32 shape = 0; shape < numShapes; shape++)
	{
		// This shape's indices
		// NOTE: assuming mesh is trangulated (which tinyobjloader does by default
		// if the mesh isn't triangulated fully), each face consists of 3 vertices
		int32 numFaces = (int32)(shapes[shape].mesh.indices.size() / 3);

		for(int32 face = 0; face < numFaces; face++)
		{
			tinyobj::index_t index0 = shapes[shape].mesh.indices[3 * face];
			tinyobj::index_t index1 = shapes[shape].mesh.indices[3 * face + 1];
			tinyobj::index_t index2 = shapes[shape].mesh.indices[3 * face + 2];

			// TODO: get texture coordinates

			// Get vertex data
			float32 vertex0[3];
			float32 vertex1[3];
			float32 vertex2[3];
			for(int32 component = 0; component < 3; component++)
			{
				// Vertex indices are separate from uv and normal indices
				int v0 = index0.vertex_index;
				int v1 = index1.vertex_index;
				int v2 = index2.vertex_index;

				if(v0 < 0 || v1 < 0 || v2 < 0)
				{
					printf("INVALID INDICES!\n");
					return false;
				}

				vertex0[component] = attrib.vertices[3 * v0 + component];
				vertex1[component] = attrib.vertices[3 * v1 + component];
				vertex2[component] = attrib.vertices[3 * v2 + component];
			}

			// Get normal data
			float32 normalv0[3];
			float32 normalv1[3];
			float32 normalv2[3];
			if(numNormals > 0)
			{
				int n0 = index0.normal_index;
				int n1 = index1.normal_index;
				int n2 = index2.normal_index;

				if(n0 < 0 || n1 < 0 || n2 < 0)
				{
					printf("INVALID NORMALS!\n");
					return false;
				}

				for(int32 component = 0; component < 3; component++)
				{
					normalv0[component] = attrib.normals[3 * n0 + component];
					normalv1[component] = attrib.normals[3 * n1 + component];
					normalv2[component] = attrib.normals[3 * n2 + component];
				}
			}
			
			// Average out all vertex normals for each face
			Vec3f n0 = CreateVec3f(normalv0[0], normalv0[1], normalv0[2]);
			Vec3f n1 = CreateVec3f(normalv1[0], normalv1[1], normalv1[2]);
			Vec3f n2 = CreateVec3f(normalv2[0], normalv2[1], normalv2[2]);
			Vec3f normal = NormalizeVec3f(n0 + n1 + n2);

			// Create triangle
			Vec3f v0 = CreateVec3f(vertex0[0], vertex0[1], vertex0[2]);
			Vec3f v1 = CreateVec3f(vertex1[0], vertex1[1], vertex1[2]);
			Vec3f v2 = CreateVec3f(vertex2[0], vertex2[1], vertex2[2]);
			Triangle tri = CreateTriangle(v0, v1, v2, normal);

			ApplyScaleToTriangle(&tri, CreateVec3f(0.3f, 0.3f, 0.3f));
			ApplyTranslationToTriangle(&tri, CreateVec3f(0.0f, 0.0f, -3.0f));
			
			tris.push_back(tri);
		}
	}

	uint32 numTris = (uint32)tris.size();
	Triangle *outTriangles = (Triangle *)malloc(numTris * sizeof(Triangle));
	for(uint32 i = 0; i < numTris; i++)
		outTriangles[i] = tris[i];

	*outTris = outTriangles;
	*numOutTris = numTris;

	printf("Finished loading .obj model!\n");
	return true;
}
