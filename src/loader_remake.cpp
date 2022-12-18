#include "loader_remake.h"
#include "defines.hpp"
#include <cgltf.h>
#include <cstdio>

#include "math/math.hpp"
#include "math/vec.hpp"
#include "scene/material.hpp"
#include "scene/triangle.hpp"

#include <stb_image.h>
#include <glad/glad.h>

bool LoadGLTF(const char *path, Array<Triangle> &out_tris, Array<MaterialGLSL> &out_mats, Mat4f &model_matrix, uint32 &texture_array)
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

		int32 num_loaded_textures = 0;
		if(num_textures > 0)
		{
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array);
			glBindTextureUnit(2, texture_array);
			glTextureStorage3D(texture_array, 1, GL_RGB32F, 256, 256, 3);
		}

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
							float *start = (float *)((uint8 *)view->buffer->data + view->offset + accessor->offset + stride * i);

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
					if (indices_accessor != nullptr &&
					   (indices_accessor->type != cgltf_type_scalar ||
						indices_accessor->component_type != cgltf_component_type_r_16u))
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

				// Load material that primitve uses
				{
					cgltf_material *material = primitive->material; (void)material;

					MaterialGLSL result_mat;

					if(material->has_pbr_metallic_roughness)
					{
						cgltf_pbr_metallic_roughness &mat_properties = material->pbr_metallic_roughness;

						cgltf_float *base_color_arr = mat_properties.base_color_factor;
						if (base_color_arr[3] < 0.99f)
						{
							printf("ERROR (glTF Loader): Currently not supporting base color with any transparency!\n");
							cgltf_free(data);
							return false;
						}

						// FIXME: This is dumb
						if(mat_properties.base_color_texture.texture != nullptr)
						{
							cgltf_image *image = mat_properties.base_color_texture.texture->image;

							void *image_data_start = (void *)((uint8 *)image->buffer_view->buffer->data + image->buffer_view->offset);
							cgltf_size len = image->buffer_view->size;

							int w = -1; int h = -1;
							int channels = -1;

							stbi_uc *image_data = stbi_load_from_memory((stbi_uc *) image_data_start, (int) len,&w, &h, &channels, 0);
							if(image_data == nullptr)
							{
								printf("ERROR (glTF Loader / Textures): Failed to load texture!\n");
								cgltf_free(data);
								return false;
							}

							// TODO: Abstract away texture loading and keep track how
							// many textures the program has actually loaded, globally
							// NOTE: Now the textures that the Display creation creates
							// are just the framebuffer and the skybox textures.
							int32 texture_index = num_loaded_textures++;

							glTextureParameteri(texture_array, GL_TEXTURE_MIN_FILTER, mat_properties.base_color_texture.texture->sampler->min_filter);
							glTextureParameteri(texture_array, GL_TEXTURE_MAG_FILTER, mat_properties.base_color_texture.texture->sampler->mag_filter);
							glTextureParameteri(texture_array, GL_TEXTURE_WRAP_S, mat_properties.base_color_texture.texture->sampler->wrap_s);
							glTextureParameteri(texture_array, GL_TEXTURE_WRAP_T, mat_properties.base_color_texture.texture->sampler->wrap_t);

							glTextureSubImage3D(texture_array, 0, 0, 0, texture_index, w, h, 1, GL_RGB, GL_UNSIGNED_BYTE, image_data);

							stbi_image_free(image_data);
						}

						if(mat_properties.metallic_factor < EPSILON)
						{
							// Material is assumed to be perfectly diffuse
							result_mat.data1 = Vec4f(base_color_arr[0], base_color_arr[1], base_color_arr[2], mat_properties.roughness_factor);

							if(mat_properties.roughness_factor > 0.0f)
								result_mat.data3.w = (float) MaterialType::MATERIAL_OREN_NAYAR;
							else
								result_mat.data3.w = (float) MaterialType::MATERIAL_LAMBERTIAN;
						}
						else
						{
							// Material is assumed to be metallic
							result_mat.data1.w = mat_properties.roughness_factor;
							result_mat.data2 = Vec4f(base_color_arr[0], base_color_arr[1], base_color_arr[2], 0.0f);
							result_mat.data3.w = (float) MaterialType::MATERIAL_SPECULAR_METAL;
						}
					}

					out_mats.append(result_mat);
				}
			}
		}

		// We need to duplicate the triangle data as the engine doesn't support indices
		for (uint32 i = 0; i <= indices.size - 3; i+= 3)
		{
			Array<Vec3f> tri_positions;
			Array<Vec3f> tri_normals;
			Array<Vec2f> tri_tex_coords;

			uint16 i0 = indices[i];
			uint16 i1 = indices[i + 1];
			uint16 i2 = indices[i + 2];

			Vec3f v0 = positions[i0];
			Vec3f v1 = positions[i1];
			Vec3f v2 = positions[i2];
			tri_positions.append(v0);
			tri_positions.append(v1);
			tri_positions.append(v2);

			Vec3f n0, n1, n2;
			if(normals.size > 0)
			{
				n0 = normals[i0];
				n1 = normals[i1];
				n2 = normals[i2];
				tri_normals.append(n0);
				tri_normals.append(n1);
				tri_normals.append(n2);
			}

			Vec2f uv0, uv1, uv2;
			if(tex_coords.size > 0)
			{
				uv0 = tex_coords[i0];
				uv1 = tex_coords[i1];
				uv2 = tex_coords[i2];
				tri_tex_coords.append(uv0);
				tri_tex_coords.append(uv1);
				tri_tex_coords.append(uv2);
			}

			Triangle tri(tri_positions, tri_normals, tri_tex_coords, 0);
			out_tris.append(tri);
		}

		// Find model matrix if any
		for (cgltf_size node_index = 0; node_index < data->nodes_count; node_index++)
		{
			cgltf_node *node = &data->nodes[node_index];
			if (node->has_matrix)
			{
				cgltf_float *m = node->matrix;
				model_matrix = Mat4f(Vec4f(m[0], m[4], m[8],  m[12]),
									 Vec4f(m[1], m[5], m[9],  m[13]),
									 Vec4f(m[2], m[6], m[10], m[14]),
									 Vec4f(m[3], m[7], m[11], m[15]));

				break;
			}
		}

		printf("--> Num loaded vertices: %u\n", positions.size);
		printf("--> Num loaded normals: %u\n", normals.size);
		printf("--> Num loaded UVs: %u\n", tex_coords.size);
		printf("--> Num loaded indices: %u\n", indices.size);
		printf("--> Num loaded tris: %u\n", out_tris.size);

		cgltf_free(data);
		return true;
	}

	return false;
}