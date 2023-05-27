#include <glad/glad.h>
#include <SDL.h>

#include "display/display.hpp"
#include "glm/trigonometric.hpp"
#include "loader.h"
#include "math/math.hpp"
#include "scene/bvh.h"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"

#include <string>

template<class T>
void PushDataToSSBO(Array<T> &data, Array<GLuint> &ssbo_array)
{
    GLuint ssbo = 0;
    if (data.size > 0)
    {
        glCreateBuffers(1, &ssbo);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_array.size, ssbo);
        glNamedBufferStorage(ssbo, (GLsizeiptr) (data.size * sizeof(T)), &(data[0]), 0);
    }

    ssbo_array.append(ssbo);
}

int main()
{
    Display display("Pathtracer", WIDTH, HEIGHT);

    Mesh mesh;

    bool isLoaded = LoadGLTF("res/models/CornellBox_lit.glb", mesh);
    if (!isLoaded)
    {
        printf("Failed to load model!\n");
        return -1;
    }

    // Apply model matrix to tris
	mesh.Translate(glm::vec3(0.0f, -2.0f, -5.0f));
	mesh.Rotate(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
	mesh.Scale(2.0f);
	mesh.ApplyModelMatrixToTris();

    Array<TriangleGLSL> unsorted_model_tris = mesh.ConvertToSSBOFormat();

    Array<TriangleGLSL> model_glsl_tris;
    Array<BVHNodeGLSL> bvh_ssbo = CalculateBVH(unsorted_model_tris, model_glsl_tris);

    // Find all emissive triangles in scene
    Array<uint32> emissive_tris(model_glsl_tris.size);
    for (uint32 i = 0; i < model_glsl_tris.size; i++)
    {
        MaterialGLSL current_mat = mesh.materials[model_glsl_tris[i].data4.w];
        if (current_mat.data3.x >= EPSILON || current_mat.data3.y >= EPSILON || current_mat.data3.z >= EPSILON)
        {
            emissive_tris.append(i);
        }
    }

    // Set up data to be passed to SSBOs

    Array<MaterialGLSL> materials_ssbo = mesh.materials;
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1000.0f), 0.0f, 0, MaterialType::MATERIAL_LIGHT));

    Array<SphereGLSL> spheres_ssbo;
	spheres_ssbo.append(SphereGLSL(glm::vec3(6.5f, 2.0f, -3.0f), 0.1f, materials_ssbo.size - 1));

	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.0f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(5.0f, 0.0f, -3.0f), 0.5f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.1f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(6.0f, 0.0f, -3.0f), 0.5f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.15f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(7.0f, 0.0f, -3.0f), 0.5f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.2f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(8.0f, 0.0f, -3.0f), 0.5f, materials_ssbo.size - 1));

    Array<uint32> emissive_spheres_ssbo;
    for (uint32 i = 0; i < spheres_ssbo.size; i++)
    {
        MaterialGLSL &mat = materials_ssbo[spheres_ssbo[i].mat_index[0]];
        if (glm::all(glm::greaterThan(mat.emitted_radiance(), glm::vec3(EPSILON))))
        {
            emissive_spheres_ssbo.append(i);
        }
    }

    Array<GLuint> ssbo_array;
    PushDataToSSBO(spheres_ssbo, ssbo_array);
    PushDataToSSBO(model_glsl_tris, ssbo_array);
    PushDataToSSBO(emissive_tris, ssbo_array);
    PushDataToSSBO(materials_ssbo, ssbo_array);
    PushDataToSSBO(bvh_ssbo, ssbo_array);
    PushDataToSSBO(emissive_spheres_ssbo, ssbo_array);

    glUseProgram(display.compute_shader.id);

    uint32 last_time = 0;
    uint32 last_report = 0;
    uint32 frame_count = 0;

    Camera cam(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.005f, 0.05f);

    constexpr uint32 num_work_groups_x = WIDTH / 8;
    constexpr uint32 num_work_groups_y = HEIGHT / 8;

    while (display.is_open)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                display.is_open = false;
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                cam.mouse_look((float) e.motion.xrel, (float) e.motion.yrel);
                frame_count = 0;
            }
        }

        uint32 current_time = SDL_GetTicks();
        uint32 delta_time = (uint32) current_time - (uint32) last_time;

        cam.move(SDL_GetKeyboardState(nullptr), delta_time, frame_count);

        glClear(GL_COLOR_BUFFER_BIT);

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "COMPUTE");
        {
            // Compute shader data and dispatch
            glUseProgram(display.compute_shader.id);

            glBindTextureUnit(1, display.cubemap_texture);

			if(mesh.texture_array != (uint32) -1)
			{
            	glBindTextureUnit(2, mesh.texture_array);
			}

            if (frame_count == 0)
            {
                CameraGLSL cam_glsl(cam.origin, cam.forward, cam.right, cam.fly_speed, cam.look_sens);
                glNamedBufferSubData(cam.cam_ubo, 0, sizeof(CameraGLSL), &cam_glsl);
            }

            glUniform3ui(0, pcg32_random(), frame_count++, BOUNCE_COUNT);

            glDispatchCompute(num_work_groups_x, num_work_groups_y, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glBindTextureUnit(2, 0);
            glBindTextureUnit(1, 0);
        }
        glPopDebugGroup();

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "SCREEN QUAD");
        {
            // Full screen quad drawing
            glUseProgram(display.render_buffer_shader.id);
            glBindTextureUnit(0, display.render_buffer_texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Clean up state
            glBindTextureUnit(0, 0);
        }
        glPopDebugGroup();

        // Frame time calculation
        if (current_time > last_report + 1000)
        {
            uint32 fps = (uint32) (1.0f / ((float) delta_time / 1000.0f));
            std::string new_title = "Pathtracer | ";
            new_title += std::to_string(delta_time) + "ms | ";
            new_title += std::to_string(fps) + "fps | ";
            new_title += std::to_string(frame_count) + " total frame count | ";

            UpdateDisplayTitle(display, new_title.c_str());
            last_report = current_time;
        }
        last_time = current_time;

        SDL_GL_SwapWindow(display.window_handle);
    }

    CloseDisplay(display);
    return 0;
}
