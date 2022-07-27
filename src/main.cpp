#include "loader.hpp"
#include "scene/bvh.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"
#include "scene/triangle.hpp"
#include "threads.hpp"

#include <cstdlib>
#include <cstring>
#include <string>

#include "display/display.hpp"
#include <glad/glad.h>
#include <SDL.h>

struct SphereGLSL
{
	Vec4f data;          // o.x, o.y, o.z, radius
	uint32 mat_index[4]; // mat_index, 0, 0, 0
};

struct TriangleGLSL
{
	Vec4f data1;     // v0.x, v0.y, v0.z, mat_index
	Vec4f data2;     // v1.x, v1.y, v1.z, 0
	Vec4f data3;     // v2.x, v2.y, v2.z, 0
};

struct MaterialGLSL
{
	Vec4f data1;  // diff.x, diff.y, diff.z, mat_type
	Vec4f data2;  // spec.x, spec.y, spec.z, n_spec
	Vec4f data3;  // Le.x, Le.y, Le.z, empty
};

struct BVHNodeGLSL
{
	Vec4f data1; // bmin.x, bmin.y, bmin.z, left/first_tri
	Vec4f data2; // bmax.x, bmax.y, bmax.z, num_tris
	Vec4f data3; // axis, 0, 0, 0
};

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;

	uint32 width = (uint32)WIDTH;
	uint32 height = (uint32)HEIGHT;
	// float aspect_atio = (float)width / (float)height;

	Display display = CreateDisplay("Pathtracer", width, height);
	InitRenderBuffer(display);
#if 0
	Array<Triangle> tris;
	Array<Material *> materials;
	if (!LoadModelFromObj("CornellBox-Original.obj", "../../res/", tris, materials))
	//if (!LoadModelFromObj("robot.obj", "../../res/", tris, materials))
	{
		return -1;
	}

	// Apply model matrix to tris
	Mat4f model_matrix = CreateIdentityMat4f();

	// for cornell box
	model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.0f, -3.5f), model_matrix); 

	// for robot
	//model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.5f, -4.f), model_matrix);
	//model_matrix = ScaleMat4f(CreateVec3f(0.2f, 0.2f, 0.2f), model_matrix);

	for (uint32 i = 0; i < tris.size; i++)
	{
		tris[i].v0 = model_matrix * tris[i].v0;
		tris[i].v1 = model_matrix * tris[i].v1;
		tris[i].v2 = model_matrix * tris[i].v2;
		tris[i].edge1 = tris[i].v1 - tris[i].v0;
		tris[i].edge2 = tris[i].v2 - tris[i].v0;
		tris[i].normal = NormalizeVec3f(Cross(tris[i].edge1, tris[i].edge2));
	}

	// Construct BVH tree and sort triangle list according to it
	Array<BVHNode> bvh_tree(2 * tris.size - 1);
	
	BVHNode root_node {};
	root_node.first_tri = 0;
	AppendToArray(bvh_tree, root_node);
	
	if (!ConstructBVHSweepSAH(tris._data, (uint32)tris.size, bvh_tree, 0))
	{
		printf("Error in BVH construction!\n");
		return -1;
	}

	// Find all emissive triangles in scene
	Array<uint32> emissive_tris(tris.size);
	uint32 num_emissive_tris = 0;
	for (uint32 i = 0; i < tris.size; i++)
	{
		Material *current_mat = materials[tris[i].mat_index];
		if (current_mat->Le.x >= EPSILON || current_mat->Le.y >= EPSILON || current_mat->Le.z >= EPSILON)
		{
			AppendToArray(emissive_tris, i);
		}
	}
#endif
	// Set up data to be passed to SSBOs

	Array<MaterialGLSL> materials_ssbo;
	AppendToArray(materials_ssbo, { CreateVec4f(0.8f / PI), CreateVec4f(0.0f), CreateVec4f(0.0f) });

	Array<SphereGLSL> spheres_ssbo;
	AppendToArray(spheres_ssbo, { CreateVec4f(1.05f, 0.0f, -4.0f, 0.3f), {0} });
	AppendToArray(spheres_ssbo, { CreateVec4f(-1.05f, 0.0f, -4.0f, 0.3f), {0} });
	AppendToArray(spheres_ssbo, { CreateVec4f(0.35f, 0.0f, -4.0f, 0.3f), {0} });
	AppendToArray(spheres_ssbo, { CreateVec4f(-0.35f, 0.0f, -4.0f, 0.3f), {0} });
	AppendToArray(spheres_ssbo, { CreateVec4f(1.05f, -0.65f, -4.0f, 0.3f), { 0 } });
	AppendToArray(spheres_ssbo, { CreateVec4f(-1.05f, -0.65f, -4.0f, 0.3f), { 0 } });
	AppendToArray(spheres_ssbo, { CreateVec4f(0.35f, -0.65f, -4.0f, 0.3f), { 0 } });
	AppendToArray(spheres_ssbo, { CreateVec4f(-0.35f, -0.65f, -4.0f, 0.3f), { 0 } });
	AppendToArray(spheres_ssbo, { CreateVec4f(0.0f, -101.0f, -4.0f, 100.0f), {0} });

#if 0
	Array<TriangleGLSL> model_tris_ssbo(tris.size);
	for (uint32 i = 0; i < tris.size; i++)
	{
		const Triangle &current_tri = tris[i];
		const Vec3f &v0 = current_tri.v0;
		const Vec3f &v1 = current_tri.v1;
		const Vec3f &v2 = current_tri.v2;

		TriangleGLSL tmp;
		tmp.data1 = CreateVec4f(v0.x, v0.y, v0.z, (float)current_tri.mat_index);
		tmp.data2 = CreateVec4f(v1.x, v1.y, v1.z, 0.0f);
		tmp.data3 = CreateVec4f(v2.x, v2.y, v2.z, 0.0f);

		AppendToArray(model_tris_ssbo, tmp);
	}
	DeallocateArray(tris);


	Array<MaterialGLSL> materials_ssbo(materials.size);
	for(uint32 i = 0; i < materials.size; i++)
	{
		const Material *current_mat = materials[i];
		const Vec3f &diff = current_mat->diffuse;
		const Vec3f &spec = current_mat->specular;
		const Vec3f &Le = current_mat->Le;

		MaterialGLSL tmp;
		tmp.data1 = CreateVec4f(diff.x, diff.y, diff.z, (float)current_mat->type);
		tmp.data2 = CreateVec4f(spec.x, spec.y, spec.z, (float)current_mat->n_spec);
		tmp.data3 = CreateVec4f(Le.x, Le.y, Le.z, 0.0f);

		AppendToArray(materials_ssbo, tmp);
	}

	for (uint32 i = 0; i < materials.size; i++)
	{
		free(materials[i]);
	}
	DeallocateArray(materials);

	Array<BVHNodeGLSL> bvh_ssbo(bvh_tree.size);
	for(uint32 i = 0; i < bvh_tree.size; i++)
	{
		const BVHNode &current_node = bvh_tree[i];
		const AABB &current_aabb = current_node.node_AABB;

		BVHNodeGLSL tmp;
		tmp.data1 = CreateVec4f(current_aabb.bmin.x, current_aabb.bmin.y, current_aabb.bmin.z, (float)current_node.first_tri);
		tmp.data2 = CreateVec4f(current_aabb.bmax.x, current_aabb.bmax.y, current_aabb.bmax.z, (float)current_node.num_tris);
		tmp.data3 = CreateVec4f((float)current_node.axis, 0.0f, 0.0f, 0.0);

		AppendToArray(bvh_ssbo, tmp);
	}
	DeallocateArray(bvh_tree);
#endif

	// Set up SSBOs
	GLuint ssbo[5];
	glCreateBuffers(5, ssbo);

	// Sphere SSBO
	if(spheres_ssbo.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);
		glNamedBufferStorage(ssbo[0], spheres_ssbo.size * sizeof(SphereGLSL), &(spheres_ssbo._data[0]), 0);
		DeallocateArray(spheres_ssbo);
	}
#if 0
	if(model_tris_ssbo.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);
		glNamedBufferStorage(ssbo[1], model_tris_ssbo.size * sizeof(TriangleGLSL), &(model_tris_ssbo[0]), 0);
		DeallocateArray(model_tris_ssbo);
	}

	if(emissive_tris.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[2]);
		glNamedBufferStorage(ssbo[2], emissive_tris.size * sizeof(uint32), &(emissive_tris[0]), 0);
		DeallocateArray(emissive_tris);
	}
#endif
	if(materials_ssbo.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[3]);
		glNamedBufferStorage(ssbo[3], materials_ssbo.size * sizeof(MaterialGLSL), &(materials_ssbo[0]), 0);
		DeallocateArray(materials_ssbo);
	}
#if 0
	if(bvh_ssbo.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[4]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[4]);
		glNamedBufferStorage(ssbo[4], bvh_ssbo.size * sizeof(BVHNodeGLSL), &(bvh_ssbo[0]), 0);
		DeallocateArray(bvh_ssbo);
	}
#endif
	glUseProgram(display.rb_shader_program);
	glUniform1i(glGetUniformLocation(display.rb_shader_program, "tex"), 0);

	glUseProgram(display.compute_shader_program);
	glUniform1i(glGetUniformLocation(display.compute_shader_program, "screen"), 0);

	uint32 last_time = 0;
	uint32 last_report = 0;
	uint32 frame_count = 0;

	glUseProgram(display.compute_shader_program);
	uint32 frame_data_location = (uint32)glGetUniformLocation(display.compute_shader_program, "u_frame_data");

	while(display.is_open)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
			{
				display.is_open = false;
			}
		}

		uint32 current_time = SDL_GetTicks();

		glClear(GL_COLOR_BUFFER_BIT);

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "COMPUTE");
		{
			// Compute shader data and dispatch
			glUseProgram(display.compute_shader_program);
			glUniform2ui(frame_data_location, pcg32_random(), frame_count++);

			const uint32 num_groups_x = width / 8;
			const uint32 num_groups_y = height / 8;
			glDispatchCompute(num_groups_x, num_groups_y, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}
		glPopDebugGroup();

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "SCREEN QUAD");
		{
			// Full screen quad drawing
			glUseProgram(display.rb_shader_program);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glPopDebugGroup();

		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR)
		{
			printf("OPENGL ERROR!\n");
		}

		// Frame time calculation
		if(current_time > last_report + 1000)
		{
			uint32 delta_time = current_time - last_time;
			uint32 fps = (uint32)(1.0f / ((float)delta_time / 1000.0f));
			std::string new_title = "Pathtracer | ";
			new_title += std::to_string(delta_time) + "ms | ";
			new_title += std::to_string(fps) + "fps | ";
			new_title += std::to_string(frame_count) + " total frame count";
			UpdateDisplayTitle(display, new_title.c_str());
			last_report = current_time;
		}
		last_time = current_time;

		SDL_GL_SwapWindow(display.window_handle);
	}

	CloseDisplay(display);
	return 0;
}
