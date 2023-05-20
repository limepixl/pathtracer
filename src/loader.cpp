#include "defines.hpp"
#include "loader.h"
#include <cgltf.h>
#include <cstdio>

#include "math/math.hpp"
#include "scene/material.hpp"
#include "scene/triangle.hpp"

#include <stb_image.h>
#include <stb_image_resize.h>
#include <glad/glad.h>

#include <glm/gtc/quaternion.hpp>

bool LoadGLTF(const char *path, Mesh &out_mesh)
{
    cgltf_options options = {};
    cgltf_data *data = nullptr;

    // Parse GLTF / GLB file with given options and put metadata into `data`
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result == cgltf_result_success)
    {
        result = cgltf_load_buffers(&options, data, path);
    }

    if (result == cgltf_result_success)
    {
        result = cgltf_validate(data);
    }

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

        constexpr uint64 texture_layer_width = 512;
        constexpr uint64 texture_layer_height = 512;

        int32 num_loaded_textures = 0;
        if (num_textures > 0)
        {
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &out_mesh.texture_array);
            glBindTextureUnit(2, out_mesh.texture_array);
            glTextureStorage3D(out_mesh.texture_array,
                               1,
                               GL_RGB32F,
                               texture_layer_width,
                               texture_layer_height,
                               (GLsizei) num_textures);
        }

        for (cgltf_size mesh_index = 0; mesh_index < num_meshes; mesh_index++)
        {
			Array<glm::vec3> positions;
			Array<glm::vec3> normals;
			Array<glm::vec2> tex_coords;
			Array<uint16> indices;

            cgltf_mesh *mesh = &data->meshes[mesh_index];
            cgltf_size num_mesh_primitives = mesh->primitives_count;

            for (cgltf_size mesh_prim_index = 0; mesh_prim_index < num_mesh_primitives; mesh_prim_index++)
            {
                cgltf_primitive *primitive = &mesh->primitives[mesh_prim_index];

                // Only supporting triangles as primitives (for now)
                if (primitive->type != cgltf_primitive_type_triangles)
                {
                    printf("ERROR (glTF Loader): Only triangles are supported as primitives!");
                    cgltf_free(data);
                    return false;
                }

                // Load attribute data that primitive uses
                for (cgltf_size attr_index = 0; attr_index < primitive->attributes_count; attr_index++)
                {
                    cgltf_attribute *attribute = &primitive->attributes[attr_index];
                    if (attribute->type == cgltf_attribute_type_position ||
                        attribute->type == cgltf_attribute_type_normal ||
                        attribute->type == cgltf_attribute_type_texcoord)
                    {
                        cgltf_accessor *accessor = attribute->data;
                        cgltf_buffer_view *view = accessor->buffer_view;
                        cgltf_size stride = view->stride != 0 ? view->stride : accessor->stride;

                        for (uint32 i = 0; i < accessor->count; i++)
                        {
                            float *start = (float *) ((uint8 *) view->buffer->data + view->offset + accessor->offset +
                                                      stride * i);

                            if (accessor->type == cgltf_type_vec3)
                            {
                                glm::vec3 vec3;
                                vec3.x = *((float *) start);
                                vec3.y = *((float *) start + 1);
                                vec3.z = *((float *) start + 2);

                                if (attribute->type == cgltf_attribute_type_position)
                                {
                                    positions.append(vec3);
                                }
                                else if (attribute->type == cgltf_attribute_type_normal)
                                {
                                    normals.append(vec3);
                                }
                                else
                                {
                                    printf("ERROR (glTF Loader): This attribute type is unsupported!\n");
                                    cgltf_free(data);
                                    return false;
                                }
                            }
                            else if (accessor->type == cgltf_type_vec2)
                            {
                                glm::vec2 vec2;
                                vec2.x = *((float *) start);
                                vec2.y = *((float *) start + 1);

                                if (attribute->type == cgltf_attribute_type_texcoord)
                                {
                                    tex_coords.append(vec2);
                                }
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
                    if (indices_accessor != nullptr && (indices_accessor->type != cgltf_type_scalar ||
                                                        indices_accessor->component_type != cgltf_component_type_r_16u))
                    {
                        printf("ERROR (glTF Loader): Indices accessor type or component type is wrong!\n");
                        cgltf_free(data);
                        return false;
                    }

                    cgltf_buffer_view *view = indices_accessor->buffer_view;
                    cgltf_size stride = view->stride != 0 ? view->stride : indices_accessor->stride;

                    for (cgltf_size i = 0; i < indices_accessor->count; i++)
                    {
                        uint16 val = *(uint16 *) ((uint8 *) view->buffer->data + view->offset + stride * i);
                        indices.append(val);
                    }
                }

                // Load material that primitive uses
				uint32 mat_index = out_mesh.materials.size;
                {
                    cgltf_material *material = primitive->material;

                    MaterialGLSL result_mat;
					result_mat.data3.w = -1.0f;

                    if (material == nullptr)
                    {
                        result_mat.data1 = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
                    }

                    else if (material->has_pbr_metallic_roughness)
                    {
                        cgltf_pbr_metallic_roughness &mat_properties = material->pbr_metallic_roughness;

                        cgltf_float *base_color_arr = mat_properties.base_color_factor;
                        if (base_color_arr[3] < 0.99f)
                        {
                            printf("ERROR (glTF Loader): Currently not supporting base color with any transparency!\n");
                            cgltf_free(data);
                            return false;
                        }

                        if (mat_properties.base_color_texture.texture != nullptr)
                        {
                            cgltf_image *image = mat_properties.base_color_texture.texture->image;

                            void *image_data_start = (void *) ((uint8 *) image->buffer_view->buffer->data +
                                                               image->buffer_view->offset);
                            cgltf_size len = image->buffer_view->size;

                            int w = -1;
                            int h = -1;
                            int channels = -1;

                            stbi_uc *image_data = stbi_load_from_memory((stbi_uc *) image_data_start,
                                                                        (int) len,
                                                                        &w,
                                                                        &h,
                                                                        &channels,
                                                                        3);
                            if (image_data == nullptr)
                            {
                                printf("ERROR (glTF Loader / Textures): Failed to load texture!\n");
                                cgltf_free(data);
                                return false;
                            }

                            if ((w != texture_layer_width || h != texture_layer_height) && channels != -1)
                            {
                                stbi_uc *resized_image_data = new stbi_uc[texture_layer_width *
                                                                          texture_layer_height *
                                                                          (uint64) channels];
                                stbir_resize_uint8(image_data, w, h, 0,
                                                   resized_image_data, 512, 512, 0, channels);

                                if (resized_image_data != nullptr)
                                {
                                    stbi_image_free(image_data);
                                    image_data = resized_image_data;
                                }
                            }

                            // TODO: Abstract away texture loading and keep track how
                            // many textures the program has actually loaded, globally
                            // NOTE: Now the textures that the Display creation creates
                            // are just the framebuffer and the skybox textures.
                            int32 texture_index = num_loaded_textures++;
                            result_mat.data3.w = (float) texture_index;

                            cgltf_sampler *sampler = mat_properties.base_color_texture.texture->sampler;
                            cgltf_int min_filter_mode = (sampler != nullptr) ? sampler->min_filter : GL_LINEAR;
                            cgltf_int mag_filter_mode = (sampler != nullptr) ? sampler->mag_filter : GL_LINEAR;
                            cgltf_int wrap_mode_s = (sampler != nullptr) ? sampler->wrap_s : GL_REPEAT;
                            cgltf_int wrap_mode_t = (sampler != nullptr) ? sampler->wrap_t : GL_REPEAT;

                            glTextureParameteri(out_mesh.texture_array, GL_TEXTURE_MIN_FILTER, min_filter_mode);
                            glTextureParameteri(out_mesh.texture_array, GL_TEXTURE_MAG_FILTER, mag_filter_mode);
                            glTextureParameteri(out_mesh.texture_array, GL_TEXTURE_WRAP_S, wrap_mode_s);
                            glTextureParameteri(out_mesh.texture_array, GL_TEXTURE_WRAP_T, wrap_mode_t);

                            glTextureSubImage3D(out_mesh.texture_array,
                                                0,
                                                0,
                                                0,
                                                texture_index,
                                                texture_layer_width,
                                                texture_layer_height,
                                                1,
                                                GL_RGB,
                                                GL_UNSIGNED_BYTE,
                                                image_data);

                            stbi_image_free(image_data);
                        }

                        if (mat_properties.metallic_factor < EPSILON)
                        {
                            // Material is assumed to be perfectly diffuse
                            result_mat.data1 = glm::vec4(base_color_arr[0],
                                                     	 base_color_arr[1],
                                                     	 base_color_arr[2],
                                                     	 mat_properties.roughness_factor);

                            if (mat_properties.roughness_factor > EPSILON)
                            {
                                result_mat.data2.w = (float) MaterialType::MATERIAL_OREN_NAYAR;
                                // NOTE: this maps [0,1] to [0, 0.35] which is only based on hearsay and not any maths
                                // as I could not find a specific resource that outlines the max realistic roughness for O-N
                                // TODO: Implement other better purely diffuse material
                                result_mat.data1.w *= 0.35f;
                            }
                            else
                            {
                                result_mat.data2.w = (float) MaterialType::MATERIAL_LAMBERTIAN;
                            }
                        }
                        else
                        {
                            // Material is assumed to be metallic
                            result_mat.data1.w = mat_properties.roughness_factor;
                            result_mat.data2 = glm::vec4(base_color_arr[0], base_color_arr[1], base_color_arr[2], (float) MaterialType::MATERIAL_SPECULAR_METAL);
                        }
                    }

                    out_mesh.materials.append(result_mat);
                }

				// We need to duplicate the triangle data as the engine doesn't support indices
				for (uint32 i = 0; i <= indices.size - 3; i += 3)
				{
					Array<glm::vec3> tri_positions;
					Array<glm::vec3> tri_normals;
					Array<glm::vec2> tri_tex_coords;

					uint16 i0 = indices[i];
					uint16 i1 = indices[i + 1];
					uint16 i2 = indices[i + 2];

					glm::vec3 v0 = positions[i0];
					glm::vec3 v1 = positions[i1];
					glm::vec3 v2 = positions[i2];
					tri_positions.append(v0);
					tri_positions.append(v1);
					tri_positions.append(v2);

					glm::vec3 n0, n1, n2;
					if (normals.size > 0)
					{
						n0 = normals[i0];
						n1 = normals[i1];
						n2 = normals[i2];
						tri_normals.append(n0);
						tri_normals.append(n1);
						tri_normals.append(n2);
					}

					glm::vec2 uv0, uv1, uv2;
					if (tex_coords.size > 0)
					{
						uv0 = tex_coords[i0];
						uv1 = tex_coords[i1];
						uv2 = tex_coords[i2];
						tri_tex_coords.append(uv0);
						tri_tex_coords.append(uv1);
						tri_tex_coords.append(uv2);
					}

					Triangle tri(tri_positions, tri_normals, tri_tex_coords, mat_index);
					out_mesh.triangles.append(tri);
				}
            }
        }

        // Find model matrix if any
        for (cgltf_size node_index = 0; node_index < data->nodes_count; node_index++)
        {
            cgltf_node *node = &data->nodes[node_index];
            if (node->has_matrix)
            {
                cgltf_float *m = node->matrix;
                out_mesh.model_matrix = glm::mat4(glm::vec4(m[0], m[4], m[8], m[12]),
												  glm::vec4(m[1], m[5], m[9], m[13]),
												  glm::vec4(m[2], m[6], m[10], m[14]),
												  glm::vec4(m[3], m[7], m[11], m[15]));

                out_mesh.ApplyModelTransform();

				continue;
            }

			if(node->has_rotation)
			{
				cgltf_float *rotation = node->rotation;
				glm::fquat quaternion(rotation[3], rotation[0], rotation[1], rotation[2]);
				out_mesh.model_matrix *= glm::mat4_cast(quaternion);
			}

				float r10 = 2.0f * (x * y + s * z);
				float r11 = 1.0f - 2.0f * (x*x + z*z);
				float r12 = 2.0f * (y * z - s * x);

				float r20 = 2.0f * (x * z - s * y);
				float r21 = 2.0f * (y * z + s * x);
				float r22 = 1.0f - 2.0f * (x*x + y*y);

				out_mesh.model_matrix = Mat4f(
					Vec4f(r00,  r01,  r02,  0.0f),
					Vec4f(r10,  r11,  r12,  0.0f),
					Vec4f(r20,  r21,  r22,  0.0f),
					Vec4f(0.0f, 0.0f, 0.0f, 1.0f)
					);
				out_mesh.ApplyModelTransform(true);
			}
        }

        printf("--> Num loaded tris: %u\n", out_mesh.triangles.size);

        cgltf_free(data);
        return true;
    }

    return false;
}