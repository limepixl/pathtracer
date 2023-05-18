#include "scene/bvh.h"
#include "math/math.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"
#include "scene/camera.hpp"
#include "scene/triangle.hpp"
#include "loader.h"

#include <SDL_events.h>
#include <string>

#include "display/display.hpp"
#include <glad/glad.h>
#include <SDL.h>

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

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    Display display("Pathtracer", WIDTH, HEIGHT);

    Mesh mesh;

    bool isLoaded = LoadGLTF("res/models/CornellBox.glb", mesh);
    if (!isLoaded)
    {
        printf("Failed to load model!\n");
        return -1;
    }

    // Apply model matrix to tris
    mesh.model_matrix = TranslationMat4f(Vec3f(0.0f, 0.0f, -2.0f), mesh.model_matrix);
    mesh.model_matrix = ScaleMat4f(Vec3f(2.0f), mesh.model_matrix);
    mesh.ApplyModelTransform();

    Array<TriangleGLSL> unsorted_model_tris = mesh.ConvertToSSBOFormat();

    Array<TriangleGLSL> model_glsl_tris;
    Array<BVHNodeGLSL> bvh_ssbo = CalculateBVH(unsorted_model_tris, model_glsl_tris);

    // Find all emissive triangles in scene
    Array<uint32> emissive_tris(model_glsl_tris.size);
    for (uint32 i = 0; i < model_glsl_tris.size; i++)
    {
        MaterialGLSL current_mat = mesh.materials[(uint32) model_glsl_tris[i].data1.w];
        if (current_mat.data3.x >= EPSILON || current_mat.data3.y >= EPSILON || current_mat.data3.z >= EPSILON)
        {
            emissive_tris.append(i);
        }
    }

    // Set up data to be passed to SSBOs

    Array<MaterialGLSL> materials_ssbo = mesh.materials;
	materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.0f), Vec3f(100.0f), 0.0f, 0.0f, 0, MaterialType::MATERIAL_LAMBERTIAN));

    Array<SphereGLSL> spheres_ssbo;
	spheres_ssbo.append(SphereGLSL(Vec3f(4.0f, 2.0f, -2.0f), 0.25f, materials_ssbo.size - 1));

    Array<uint32> emissive_spheres_ssbo;
    for (uint32 i = 0; i < spheres_ssbo.size; i++)
    {
        MaterialGLSL &mat = materials_ssbo[spheres_ssbo[i].mat_index[0]];
        if (mat.emitted_radiance() > Vec3f(EPSILON))
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

    Camera cam(Vec3f(0.0f), Vec3f(0.0f, 0.0f, -1.0f), Vec3f(1.0f, 0.0f, 0.0f), 0.005f, 0.05f);

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
			// TODO: add separate compute shaders for separate modes
			/*
            else if (e.type == SDL_KEYDOWN)
            {
                bool switched = false;
                switch (e.key.keysym.sym)
                {
                    case SDLK_1:
                        view_mode = ViewMode::BRDF_IMPORTANCE_SAMPLING;
                        switched = true;
                        break;
                    case SDLK_2:
                        view_mode = ViewMode::NEXT_EVENT_ESTIMATION;
                        switched = true;
                        break;
                    case SDLK_3:
                        view_mode = ViewMode::MULTIPLE_IMPORTANCE_SAMPLING_BRDF_NEE;
                        switched = true;
                        break;
                    default:
                        break;
                }

                if (switched)
                {
                    frame_count = 0;
                    last_report = 0;
                }
            }
            */
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
            glBindTextureUnit(2, mesh.texture_array);

            if (frame_count == 0)
            {
                CameraGLSL cam_glsl(cam.origin, cam.forward, cam.right, cam.fly_speed, cam.look_sens);
                glNamedBufferSubData(cam.cam_ubo, 0, sizeof(CameraGLSL), &cam_glsl);
            }

            glUniform2ui(0, pcg32_random(), frame_count++);

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

			/*
            if (view_mode == ViewMode::BRDF_IMPORTANCE_SAMPLING)
            {
                new_title += std::string("BRDF importance sampling");
            }
            else if (view_mode == ViewMode::NEXT_EVENT_ESTIMATION)
            {
                new_title += std::string("Next Event Estimation");
            }
            else if (view_mode == ViewMode::MULTIPLE_IMPORTANCE_SAMPLING_BRDF_NEE)
            {
                new_title += std::string("Multiple Importance Sampling (MIS): BRDF+NEE");
            }
            */

            UpdateDisplayTitle(display, new_title.c_str());
            last_report = current_time;
        }
        last_time = current_time;

        SDL_GL_SwapWindow(display.window_handle);
    }

    CloseDisplay(display);
    return 0;
}
