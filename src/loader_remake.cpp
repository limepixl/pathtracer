#include "loader_remake.h"
#include "defines.hpp"
#include <cstdio>

#include "core/array.hpp"
#include "math/vec.hpp"

bool LoadGLTF(const char *path)
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

		printf("Loaded GLTF data from path: %s\n", path);
		printf("--> Number of meshes: %zu\n", num_meshes);
		printf("--> Number of buffers: %zu\n", num_buffers);
		printf("--> Number of buffer views: %zu\n", num_buffer_views);
		printf("--> Number of textures: %zu\n", num_textures);

		Array<Vec3f> positions;
		Array<Vec3f> normals;
		Array<Vec2f> tex_coords;

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
						for (uint32 i = 0; i < accessor->count; i++)
						{
							cgltf_buffer_view *view = accessor->buffer_view;
							cgltf_size stride = view->stride != 0 ? view->stride : accessor->stride;
							float *start = (float *)((uint8 *)view->buffer->data + stride * i);

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
			}
		}

		printf("--- Num loaded vertices: %u\n", positions.size);
		printf("--- Num loaded normals: %u\n", normals.size);
		printf("--- Num loaded texture coordinates: %u\n", tex_coords.size);

		return true;
	}

	return false;
}