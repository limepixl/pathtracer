#include "core/utils.h"
#include "display/display.hpp"
#include "glm/trigonometric.hpp"
#include "loader.h"
#include "math/math.hpp"
#include "scene/bvh.h"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    Display display("Pathtracer", WIDTH, HEIGHT, FRAMERATE);

	Model model;
    if (!LoadGLTF("res/models/CornellBox_lit.glb", model))
    {
        printf("Failed to load model!\n");
        return -1;
    }

    // Apply model matrix to tris
	model.Translate(glm::vec3(0.0f, -2.0f, -6.0f));
	model.Rotate(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
	model.Scale(2.0f);
	model.ApplyModelMatrixToTris();

    Array<TriangleGLSL> unsorted_model_tris = model.ConvertToSSBOFormat();

    Array<TriangleGLSL> model_glsl_tris;
    Array<BVHNodeGLSL> bvh_ssbo = CalculateBVH(unsorted_model_tris, model_glsl_tris);

    // Set up data to be passed to SSBOs

    Array<MaterialGLSL> materials_ssbo = model.materials;
//	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1000.0f), 0.0f, 0, MaterialType::MATERIAL_LIGHT));

    Array<SphereGLSL> spheres_ssbo;
//	spheres_ssbo.append(SphereGLSL(glm::vec3(6.5f, 2.0f, -3.0f), 0.1f, materials_ssbo.size - 1));

	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.0f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(-1.0f, 1.0f, -5.0f), 0.3f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.1f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(-0.4f, 1.0f, -5.0f), 0.3f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.15f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(0.2f, 1.0f, -5.0f), 0.3f, materials_ssbo.size - 1));
	materials_ssbo.append(MaterialGLSL(glm::vec3(0.0f), glm::vec3(0.944f, 0.776f, 0.373f), glm::vec3(0.0f), 0.2f, -1, MaterialType::MATERIAL_SPECULAR_METAL));
	spheres_ssbo.append(SphereGLSL(glm::vec3(0.8f, 1.0f, -5.0f), 0.3f, materials_ssbo.size - 1));

	// Find all emissive primitives in scene
	Array<uint32> emissive_tris = FindEmissiveTris(model_glsl_tris, materials_ssbo);
    Array<uint32> emissive_spheres_ssbo = FindEmissiveSpheres(spheres_ssbo, materials_ssbo);

    Array<GLuint> ssbo_array;
    PushDataToSSBO(spheres_ssbo, ssbo_array);
    PushDataToSSBO(model_glsl_tris, ssbo_array);
    PushDataToSSBO(emissive_tris, ssbo_array);
    PushDataToSSBO(materials_ssbo, ssbo_array);
    PushDataToSSBO(bvh_ssbo, ssbo_array);
    PushDataToSSBO(emissive_spheres_ssbo, ssbo_array);

    glUseProgram(display.compute_shader.id);

    Camera cam(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.005f, 0.05f);

    while (display.is_open)
    {
        display.ProcessEvents(cam);
		display.FrameStartMarker();

        cam.move(display.keyboard_state, display.delta_time, display.frame_count);

        glClear(GL_COLOR_BUFFER_BIT);

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "COMPUTE");
        {
            // Compute shader data and dispatch
            glUseProgram(display.compute_shader.id);

            glBindTextureUnit(1, display.cubemap_texture);

			if(model.texture_array != (uint32) -1)
			{
            	glBindTextureUnit(2, model.texture_array);
			}

            if (display.frame_count == 0)
            {
                CameraGLSL cam_glsl(cam.origin, cam.forward, cam.right, cam.fly_speed, cam.look_sens);
                glNamedBufferSubData(cam.cam_ubo, 0, sizeof(CameraGLSL), &cam_glsl);
            }

			// Update frame data UBO
			FrameData frame_data { pcg32_random(), display.frame_count++, BOUNCE_COUNT, 0 };
			glNamedBufferSubData(display.frame_data_ubo, 0, sizeof(FrameData), &frame_data);

            glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, 1);
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

		display.FrameEndMarker();
    }

    display.CloseDisplay();
    return 0;
}
