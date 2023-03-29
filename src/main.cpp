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

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    uint32 width = (uint32) WIDTH;
    uint32 height = (uint32) HEIGHT;

    Display display = CreateDisplay("Pathtracer", width, height);
    InitRenderBuffer(display);

    Mesh mesh;

    bool isLoaded = LoadGLTF("res/models/Cube.glb", mesh);
    if (!isLoaded)
    {
        printf("Failed to load model!\n");
        return -1;
    }

    // Apply model matrix to tris
//	mesh.model_matrix = ScaleMat4f(Vec3f(0.05f), model_matrix);
    mesh.model_matrix = TranslationMat4f(Vec3f(0.0f, 0.0f, -3.0f), mesh.model_matrix);

    Array<TriangleGLSL> model_tris_ssbo(mesh.triangles.size);
    for (uint32 i = 0; i < mesh.triangles.size; i++)
    {
        Triangle &current_tri = mesh.triangles[i];
        current_tri.v0 = mesh.model_matrix * current_tri.v0;
        current_tri.v1 = mesh.model_matrix * current_tri.v1;
        current_tri.v2 = mesh.model_matrix * current_tri.v2;

        Vec3f &v0 = current_tri.v0;
        Vec3f &v1 = current_tri.v1;
        Vec3f &v2 = current_tri.v2;
        Vec2f &uv0 = current_tri.uv0;
        Vec2f &uv1 = current_tri.uv1;
        Vec2f &uv2 = current_tri.uv2;

        TriangleGLSL tmp {};
        tmp.data1 = Vec4f(v0.x, v0.y, v0.z, (float) current_tri.mat_index);
        tmp.data2 = Vec4f(v1.x, v1.y, v1.z, 0.0f);
        tmp.data3 = Vec4f(v2.x, v2.y, v2.z, 0.0f);
        tmp.data4 = Vec4f(uv0.x, uv0.y, uv1.x, uv1.y);
        tmp.data5 = Vec4f(uv2.x, uv2.y, 0.0f, 0.0f);

        model_tris_ssbo.append(tmp);
    }

#if 0
    // Construct BVH tree and sort triangle list according to it
    Array<BVHNode> bvh_tree(2 * model_tris.size - 1);

    BVHNode root_node {};
    root_node.first_tri = 0;
    bvh_tree.append(root_node);

    if (!ConstructBVHObjectMedian(model_tris._data, model_tris.size, bvh_tree, 0))
    {
        printf("Error in BVH construction!\n");
        return -1;
    }
#endif

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

#if 0
    // Set up data to be passed to SSBOs

    Array<MaterialGLSL> materials_ssbo;
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.0f), Vec3f(10000.0f), 0.0f, 0.0f, MaterialType::MATERIAL_LAMBERTIAN));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.0f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.025f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.075f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.1f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.125f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.15f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.175f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));
    materials_ssbo.append(MaterialGLSL(Vec3f(0.0f), Vec3f(0.944f, 0.776f, 0.373f), Vec3f(0.0f), 0.2f, 0.0f, MaterialType::MATERIAL_SPECULAR_METAL));

    materials_ssbo.append(MaterialGLSL(Vec3f(0.95f), Vec3f(0.0f), Vec3f(0.0f), 0.0f, 0.0f, MaterialType::MATERIAL_OREN_NAYAR));

    Array<SphereGLSL> spheres_ssbo;

    // Grid of spheres
    spheres_ssbo.append(SphereGLSL(Vec3f(-0.99f,  0.33f, -3.0f), 0.33f, 1));
    spheres_ssbo.append(SphereGLSL(Vec3f(-0.33f,  0.33f, -3.0f), 0.33f, 2));
    spheres_ssbo.append(SphereGLSL(Vec3f( 0.33f,  0.33f, -3.0f), 0.33f, 3));
    spheres_ssbo.append(SphereGLSL(Vec3f( 0.99f,  0.33f, -3.0f), 0.33f, 4));
    spheres_ssbo.append(SphereGLSL(Vec3f(-0.99f, -0.33f, -3.0f), 0.33f, 5));
    spheres_ssbo.append(SphereGLSL(Vec3f(-0.33f, -0.33f, -3.0f), 0.33f, 6));
    spheres_ssbo.append(SphereGLSL(Vec3f( 0.33f, -0.33f, -3.0f), 0.33f, 7));
    spheres_ssbo.append(SphereGLSL(Vec3f( 0.99f, -0.33f, -3.0f), 0.33f, 8));

    // Point light and sphere next to it
    spheres_ssbo.append(SphereGLSL(Vec3f(0.0f, 1.0f, -3.0f), 0.3f, 9));
//	spheres_ssbo.append(SphereGLSL(Vec3f(0.0f, 0.0f, 1.0f), 0.03f, 0));
#endif

#if 0
    Array<BVHNodeGLSL> bvh_ssbo(bvh_tree.size);
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
#endif

#if 0
    Array<uint32> emissive_spheres_ssbo;
    for (uint32 i = 0; i < spheres_ssbo.size; i++)
    {
        MaterialGLSL &mat = materials_ssbo[spheres_ssbo[i].mat_index[0]];
        if (mat.data3.x > EPSILON || mat.data3.y > EPSILON || mat.data3.z > EPSILON)
        {
            emissive_spheres_ssbo.append(i);
        }
    }
#endif

    // Set up SSBOs
    GLuint ssbo[6];
    glCreateBuffers(6, ssbo);

#if 0
    // Sphere SSBO
    if(spheres_ssbo.size > 0)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);
        glNamedBufferStorage(ssbo[0], (GLsizeiptr)(spheres_ssbo.size * sizeof(SphereGLSL)), &(spheres_ssbo._data[0]), 0);
    }
#endif

    if (model_tris_ssbo.size > 0)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);
        glNamedBufferStorage(ssbo[1],
                             (GLsizeiptr) (model_tris_ssbo.size * sizeof(TriangleGLSL)),
                             &(model_tris_ssbo[0]),
                             0);
    }

    if (emissive_tris.size > 0)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[2]);
        glNamedBufferStorage(ssbo[2], (GLsizeiptr) (emissive_tris.size * sizeof(uint32)), &(emissive_tris[0]), 0);
    }

    if (mesh.materials.size > 0)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[3]);
        glNamedBufferStorage(ssbo[3],
                             (GLsizeiptr) (mesh.materials.size * sizeof(MaterialGLSL)),
                             &(mesh.materials[0]),
                             0);
    }

#if 0
        if(bvh_ssbo.size > 0)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[4]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[4]);
            glNamedBufferStorage(ssbo[4], (GLsizeiptr)(bvh_ssbo.size * sizeof(BVHNodeGLSL)), &(bvh_ssbo[0]), 0);
        }
#endif

#if 0
        if (emissive_spheres_ssbo.size > 0)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[5]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo[5]);
            glNamedBufferStorage(ssbo[5], (GLsizeiptr)(emissive_spheres_ssbo.size * sizeof(uint32)), &(emissive_spheres_ssbo[0]), 0);
        }
#endif

    glUseProgram(display.compute_shader_program);

    uint32 last_time = 0;
    uint32 last_report = 0;
    uint32 frame_count = 0;
    uint32 view_mode = 1;

    Camera cam(
            Vec3f(0.0f),
            Vec3f(0.0f, 0.0f, -1.0f),
            Vec3f(1.0f, 0.0f, 0.0f),
            0.005f,
            0.05f);

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
                switch (e.key.keysym.sym)
                {
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                        view_mode = e.key.keysym.sym - '0';
                        frame_count = 0;
                        last_report = 0;
                        break;
                    default:
                        break;
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
            glUseProgram(display.compute_shader_program);

            glBindTextureUnit(1, display.cubemap_texture);
            glBindTextureUnit(2, mesh.texture_array);

            if (frame_count == 0)
            {
                CameraGLSL cam_glsl(cam.origin, cam.forward, cam.right, cam.fly_speed, cam.look_sens);
                glNamedBufferSubData(cam.cam_ubo, 0, sizeof(CameraGLSL), &cam_glsl);
            }

            glUniform3ui(0, pcg32_random(), frame_count++, view_mode);

            const uint32 num_groups_x = width / 8;
            const uint32 num_groups_y = height / 8;
            glDispatchCompute(num_groups_x, num_groups_y, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glBindTextureUnit(2, 0);
            glBindTextureUnit(1, 0);
        }
        glPopDebugGroup();

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "SCREEN QUAD");
        {
            // Full screen quad drawing
            glUseProgram(display.rb_shader_program);
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

            if (view_mode == 1)
            {
                new_title += std::string("BRDF importance sampling");
            }
            else if (view_mode == 2)
            {
                new_title += std::string("Next Event Estimation");
            }
            else
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
