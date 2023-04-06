#include "scene/bvh.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"
#include "scene/camera.hpp"
#include "scene/triangle.hpp"

#include <SDL_events.h>
#include <string>

#include "display/display.hpp"
#include <glad/glad.h>
#include <SDL.h>

#include "loader.h"

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

    bool isLoaded = LoadGLTF("res/models/Avocado.glb", mesh);
    if (!isLoaded)
    {
        printf("Failed to load model!\n");
        return -1;
    }

    // Apply model matrix to tris
    mesh.model_matrix = TranslationMat4f(Vec3f(0.0f, 0.0f, -2.0f), mesh.model_matrix);
    mesh.model_matrix = ScaleMat4f(Vec3f(5.0f, 5.0f, 5.0f), mesh.model_matrix);
    mesh.ApplyModelTransform();

    Array<TriangleGLSL> model_tris_ssbo = mesh.ConvertToSSBOFormat();

    // Construct BVH tree and sort triangle list according to it
    Array<BVHNode> bvh_tree(2 * model_tris_ssbo.size - 1);

    BVHNode root_node {};
    root_node.first_tri = 0;
    bvh_tree.append(root_node);

    if (!ConstructBVHObjectMedian(mesh.triangles._data, mesh.triangles.size, bvh_tree, 0))
    {
        printf("Error in BVH construction!\n");
        return -1;
    }

    // Find all emissive triangles in scene
    Array<uint32> emissive_tris(mesh.triangles.size);
    for (uint32 i = 0; i < mesh.triangles.size; i++)
    {
        MaterialGLSL current_mat = mesh.materials[mesh.triangles[i].mat_index];
        if (current_mat.data3.x >= EPSILON || current_mat.data3.y >= EPSILON || current_mat.data3.z >= EPSILON)
        {
            emissive_tris.append(i);
        }
    }

    // Set up data to be passed to SSBOs

    Array<MaterialGLSL> materials_ssbo;
    Array<SphereGLSL> spheres_ssbo;

    Array<BVHNodeGLSL> bvh_ssbo;
    for(uint32 i = 0; i < bvh_tree.size; i++)
    {
        const BVHNode &current_node = bvh_tree[i];
        const AABB &current_aabb = current_node.node_AABB;

        BVHNodeGLSL tmp;
        tmp.data1 = Vec4f(current_aabb.bmin.x, current_aabb.bmin.y, current_aabb.bmin.z, (float)current_node.first_tri);
        tmp.data2 = Vec4f(current_aabb.bmax.x, current_aabb.bmax.y, current_aabb.bmax.z, (float)current_node.num_tris);
        tmp.data3 = Vec4f((float)current_node.axis, 0.0f, 0.0f, 0.0);

        bvh_ssbo.append(tmp);
    }

    Array<uint32> emissive_spheres_ssbo;
    for (uint32 i = 0; i < spheres_ssbo.size; i++)
    {
        MaterialGLSL &mat = materials_ssbo[spheres_ssbo[i].mat_index[0]];
        if (mat.data3.x > EPSILON || mat.data3.y > EPSILON || mat.data3.z > EPSILON)
        {
            emissive_spheres_ssbo.append(i);
        }
    }

    Array<GLuint> ssbo_array;
    PushDataToSSBO(spheres_ssbo, ssbo_array);
    PushDataToSSBO(model_tris_ssbo, ssbo_array);
    PushDataToSSBO(emissive_tris, ssbo_array);
    PushDataToSSBO(mesh.materials, ssbo_array);
    PushDataToSSBO(bvh_ssbo, ssbo_array);
    PushDataToSSBO(emissive_spheres_ssbo, ssbo_array);

    glUseProgram(display.compute_shader.id);

    uint32 last_time = 0;
    uint32 last_report = 0;
    uint32 frame_count = 0;
    ViewMode view_mode = ViewMode::MULTIPLE_IMPORTANCE_SAMPLING_BRDF_NEE;

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

            glUniform3ui(0, pcg32_random(), frame_count++, (GLuint) view_mode);

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

            UpdateDisplayTitle(display, new_title.c_str());
            last_report = current_time;
        }
        last_time = current_time;

        SDL_GL_SwapWindow(display.window_handle);
    }

    CloseDisplay(display);
    return 0;
}
