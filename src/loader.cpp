#include "loader.hpp"
#include "defines.hpp"
#include "math/math.hpp"
#include "scene/material.hpp"
#include "scene/triangle.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../thirdparty/tinyobjloader/tiny_obj_loader.h"
#include <string>

bool LoadModelFromObj(const char *fileName, const char *path,
					  Array<struct Triangle> &outTris,
					  Array<struct Material *> &outMaterials)
{
	printf("Loading %s model from path: %s\n", fileName, path);

	tinyobj::attrib_t attrib = {};
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	std::string finalPath = std::string(path) + std::string(fileName);
	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, finalPath.c_str(), path);

	// Print out any warnings or errors that tinyobjloader returned
	if (!warning.empty())
		printf("WARNING: %s\n", warning.c_str());

	if (!error.empty())
		printf("ERROR: %s\n", error.c_str());

	if (!res)
	{
		printf("Failed to load obj file at path: %s\n", finalPath.c_str());
		return false;
	}

	// int64 numVertices = (int64)(attrib.vertices.size() / 3);
	int64 numNormals = (int64)(attrib.normals.size() / 3);
	// int64 numUVs = (int64)(attrib.texcoords.size() / 2);
	int32 numMaterials = (int32)(materials.size());
	uint32 numShapes = (uint32)(shapes.size());

	// Initialize out materials
	int32 numLoadedMaterials = 0;
	outMaterials = CreateArray<Material *>(numMaterials);

	// Keep track of emissive triangles so that we can keep them as
	// light sources for NEE later on
	std::vector<uint32> emissiveTris;

	// TODO: compute AABB while loading

	std::vector<Triangle> tris;
	for (uint32 shape = 0; shape < numShapes; shape++)
	{
		// This shape's indices
		// NOTE: assuming mesh is trangulated (which tinyobjloader does by default
		// if the mesh isn't triangulated fully), each face consists of 3 vertices
		int32 numFaces = (int32)(shapes[shape].mesh.indices.size() / 3);

		for (int32 face = 0; face < numFaces; face++)
		{
			tinyobj::index_t index0 = shapes[shape].mesh.indices[3 * face];
			tinyobj::index_t index1 = shapes[shape].mesh.indices[3 * face + 1];
			tinyobj::index_t index2 = shapes[shape].mesh.indices[3 * face + 2];

			// TODO: get texture coordinates

			// Get vertex data
			float vertex0[3];
			float vertex1[3];
			float vertex2[3];
			for (int32 component = 0; component < 3; component++)
			{
				// Vertex indices are separate from uv and normal indices
				int v0 = index0.vertex_index;
				int v1 = index1.vertex_index;
				int v2 = index2.vertex_index;

				if (v0 < 0 || v1 < 0 || v2 < 0)
				{
					printf("INVALID INDICES!\n");
					return false;
				}

				vertex0[component] = attrib.vertices[3 * v0 + component];
				vertex1[component] = attrib.vertices[3 * v1 + component];
				vertex2[component] = attrib.vertices[3 * v2 + component];
			}

			// Get normal data
			Vec3f nv0 {}, nv1 {}, nv2 {};
			float normalv0[3];
			float normalv1[3];
			float normalv2[3];
			if (numNormals > 0)
			{
				int n0 = index0.normal_index;
				int n1 = index1.normal_index;
				int n2 = index2.normal_index;

				if (n0 < 0)
					n0 += numNormals;

				if (n1 < 0)
					n1 += numNormals;

				if (n2 < 0)
					n2 += numNormals;

				for (int32 component = 0; component < 3; component++)
				{
					normalv0[component] = attrib.normals[3 * n0 + component];
					normalv1[component] = attrib.normals[3 * n1 + component];
					normalv2[component] = attrib.normals[3 * n2 + component];

					// Average out all vertex normals for each face
					nv0 = CreateVec3f(normalv0[0], normalv0[1], normalv0[2]);
					nv1 = CreateVec3f(normalv1[0], normalv1[1], normalv1[2]);
					nv2 = CreateVec3f(normalv2[0], normalv2[1], normalv2[2]);
				}
			}
			else
			{
				// If there are no provided normals by the OBJ model,
				// then assume CCW winding order and calcualte the normals.
				// This makes each face have effectively one normal, so they
				// won't be interpolated between adjacent faces / shared vertices.

				Vec3f v0 = CreateVec3f(vertex0[0], vertex0[1], vertex0[2]);
				Vec3f v1 = CreateVec3f(vertex1[0], vertex1[1], vertex1[2]);
				Vec3f v2 = CreateVec3f(vertex2[0], vertex2[1], vertex2[2]);

				Vec3f edge1 = v1 - v0;
				Vec3f edge2 = v2 - v0;

				Vec3f n = Cross(edge1, edge2);
				nv0 = n;
				nv1 = n;
				nv2 = n;
			}
			Vec3f normal = NormalizeVec3f(nv0 + nv1 + nv2);

			// Materials
			int32 matID = shapes[shape].mesh.material_ids[face];
			tinyobj::material_t mat = materials[matID];
			Material *triangle_mat = nullptr;

			bool unique = true;
			for (int32 m = 0; m < numLoadedMaterials; m++)
			{
				if (!strcmp(mat.name.c_str(), outMaterials[m]->name))
				{
					unique = false;
					triangle_mat = outMaterials[m];
					break;
				}
			}

			if (unique)
			{
				// Vec3f ambient = CreateVec3f(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
				Vec3f diffuse = CreateVec3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
				Vec3f specular = CreateVec3f(mat.specular[0], mat.specular[1], mat.specular[2]);
				Vec3f emission = CreateVec3f(mat.emission[0], mat.emission[1], mat.emission[2]);

				// Purely diffuse (Lambertian) material
				if (mat.illum == 1 || (mat.illum == 2 && specular == CreateVec3f(0.0f)))
				{
					Material *tmp_mat = (Material *)malloc(sizeof(Material));
					*tmp_mat = CreateMaterial(MaterialType::MATERIAL_LAMBERTIAN,
											  diffuse,
											  specular,
											  mat.shininess,
											  emission,
											  mat.name.c_str());

					outMaterials[numLoadedMaterials++] = tmp_mat;
					triangle_mat = tmp_mat;
				}

				// Blinn-Phong BRDF with Lambertian diffuse
				else if (mat.illum == 2)
				{
					Material *tmp_mat = (Material *)malloc(sizeof(Material));
					*tmp_mat = CreateMaterial(MaterialType::MATERIAL_PHONG,
											  diffuse,
											  specular,
											  mat.shininess,
											  emission,
											  mat.name.c_str());

					outMaterials[numLoadedMaterials++] = tmp_mat;
					triangle_mat = tmp_mat;
				}

				// Reflection
				// TODO: fix hack
				else if (mat.illum == 5 || (mat.illum == 2 && specular == CreateVec3f(1.0f)))
				{
					Material *tmp_mat = (Material *)malloc(sizeof(Material));
					*tmp_mat = CreateMaterial(MaterialType::MATERIAL_IDEAL_REFLECTIVE,
											  diffuse,
											  specular,
											  mat.shininess,
											  emission,
											  mat.name.c_str());

					outMaterials[numLoadedMaterials++] = tmp_mat;
					triangle_mat = tmp_mat;
				}
			}

			// Create triangle
			Vec3f v0 = CreateVec3f(vertex0[0], vertex0[1], vertex0[2]);
			Vec3f v1 = CreateVec3f(vertex1[0], vertex1[1], vertex1[2]);
			Vec3f v2 = CreateVec3f(vertex2[0], vertex2[1], vertex2[2]);
			Triangle tri = CreateTriangle(v0, v1, v2, normal, triangle_mat);

			if (triangle_mat->Le >= CreateVec3f(0.1f, 0.1f, 0.1f))
			{
				emissiveTris.push_back((uint32)tris.size());
			}

			tris.push_back(tri);
		}
	}

	outTris = CreateArray<Triangle>((unsigned int)tris.size());
	for (uint32 i = 0; i < tris.size(); i++)
		AppendToArray(outTris, tris[i]);

	printf("Finished loading .obj model!\n");
	return true;
}
