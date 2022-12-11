#include "loader_remake.h"
#include "defines.hpp"
#include <cgltf.h>
#include <cstdio>

#include "math/vec.hpp"
#include "scene/triangle.hpp"

bool LoadGLTF(const char *path, Array<Triangle> &out_tris)
{
	cgltf_options options = {};
	cgltf_data *data = nullptr;

	// Parse GLTF / GLB file with given options and put metadata into `data`
	cgltf_result result = cgltf_parse_file(&options, path, &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, path);

	if (result == cgltf_result_success)
		result = cgltf_validate(data);

	if (result == cgltf_result_success)
	{
		cgltf_size num_meshes = data->meshes_count;
		cgltf_size num_buffers = data->buffers_count;
		cgltf_size num_buffer_views = data->buffer_views_count;
		cgltf_size num_textures = data->textures_count;

		printf("Loading glTF data from path: %s\n", path);
		printf("--> Number of meshes: %zu\n", num_meshes);
		printf("--> Number of buffers: %zu\n", num_buffers);
		printf("--> Number of buffer views: %zu\n", num_buffer_views);
		printf("--> Number of textures: %zu\n", num_textures);

		Array<Vec3f> positions;
		Array<Vec3f> normals;
		Array<Vec2f> tex_coords;
		Array<uint16> indices;

		for (cgltf_size mesh_index = 0; mesh_index < num_meshes; mesh_index++)
		{
			cgltf_mesh *mesh = &data->meshes[mesh_index];
			cgltf_size num_mesh_primitives = mesh->primitives_count;

			for(cgltf_size mesh_prim_index = 0;
				mesh_prim_index < num_mesh_primitives;
				mesh_prim_index++)
			{
				cgltf_primitive *primitive = &mesh->primitives[mesh_prim_index];

				// Only supporting triangles as primitives (for now)
				if(primitive->type != cgltf_primitive_type_triangles)
				{
					printf("ERROR (glTF Loader): Only triangles are supported as primitives!");
					cgltf_free(data);
					return false;
				}

				// Load attribute data that primitive uses
				for (cgltf_size attr_index = 0;
					 attr_index < primitive->attributes_count;
					 attr_index++)
				{
					cgltf_attribute *attribute = &primitive->attributes[attr_index];
					if( attribute->type == cgltf_attribute_type_position ||
						attribute->type == cgltf_attribute_type_normal   ||
						attribute->type == cgltf_attribute_type_texcoord)
					{
						cgltf_accessor *accessor = attribute->data;
						cgltf_buffer_view *view = accessor->buffer_view;
						cgltf_size stride = view->stride != 0 ? view->stride : accessor->stride;

						for (uint32 i = 0; i < accessor->count; i++)
						{
							float *start = (float *)((uint8 *)view->buffer->data + view->offset + stride * i);

							if(accessor->type == cgltf_type_vec3)
							{
								Vec3f vec3;
								vec3.x = *((float *)start);
								vec3.y = *((float *)start + 1);
								vec3.z = *((float *)start + 2);

								if (attribute->type == cgltf_attribute_type_position)
									positions.append(vec3);
								else if (attribute->type == cgltf_attribute_type_normal)
									normals.append(vec3);
								else
								{
									printf("ERROR (glTF Loader): This attribute type is unsupported!\n");
									cgltf_free(data);
									return false;
								}
							}
							else if(accessor->type == cgltf_type_vec2)
							{
								Vec2f vec2;
								vec2.x = *((float *)start);
								vec2.y = *((float *)start + 1);

								if(attribute->type == cgltf_attribute_type_texcoord)
									tex_coords.append(vec2);
								else
								{
									printf("ERROR (glTF Loader): This attribute type is unsupported!\n");
									cgltf_free(data);
									return false;
								}
							}
						}
					}
				}

				// Load indices that primitive uses
				{
					cgltf_accessor *indices_accessor = primitive->indices;
					if (indices_accessor->type != cgltf_type_scalar ||
						indices_accessor->component_type != cgltf_component_type_r_16u)
					{
						printf("ERROR (glTF Loader): Indices accessor type or component type is wrong!\n");
						cgltf_free(data);
						return false;
					}

					cgltf_buffer_view *view = indices_accessor->buffer_view;
					cgltf_size stride = view->stride != 0 ? view->stride : indices_accessor->stride;

					for (uint32 i = 0; i < indices_accessor->count; i++)
					{
						uint16 val = *(uint16 *)((uint8 *)view->buffer->data + view->offset + stride * i);
						indices.append(val);
					}
				}
			}
		}

		// We need to duplicate the triangle data as the engine doesn't support indices
		Array<Vec3f> indexed_positions;
		Array<Vec3f> indexed_normals;
		Array<Vec2f> indexed_tex_coords;

		for (uint32 i = 0; i < indices.size; i+= 3)
		{
			uint16 i0 = indices[i];
			uint16 i1 = indices[i + 1];
			uint16 i2 = indices[i + 2];

			Vec3f v0 = positions[i0];
			Vec3f v1 = positions[i1];
			Vec3f v2 = positions[i2];
			indexed_positions.append(v0);
			indexed_positions.append(v1);
			indexed_positions.append(v2);

			Vec3f n0 = normals[i0];
			Vec3f n1 = normals[i1];
			Vec3f n2 = normals[i2];
			indexed_normals.append(n0);
			indexed_normals.append(n1);
			indexed_normals.append(n2);

//			Vec2f uv0 = tex_coords[i0];
//			Vec2f uv1 = tex_coords[i1];
//			Vec2f uv2 = tex_coords[i2];
//			indexed_tex_coords.append(uv0);
//			indexed_tex_coords.append(uv1);
//			indexed_tex_coords.append(uv2);

			// TODO: Material index
			// TODO: Use loaded normals if available
			Triangle tri = CreateTriangle(v0, v1, v2, 0);
			out_tris.append(tri);
		}

		printf("--> Num loaded vertices: %u\n", positions.size);
		printf("--> Num loaded normals: %u\n", normals.size);
		printf("--> Num loaded UVs: %u\n", tex_coords.size);
		printf("--> Num loaded indices: %u\n", indices.size);
		printf("--> Num loaded tris: %u\n", out_tris.size);

		return true;
	}

	return false;
}