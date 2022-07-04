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

#define NUM_MAX_MATERIALS 16

struct SphereGLSL
{
	Vec4f data;          // o.x, o.y, o.z, radius
	uint32 mat_index[4]; // mat_index, 0, 0, 0
};

struct TriangleGLSL
{
	Vec4f v0v1;   // v0.x, v0.y, v0.z, v1.x
	Vec4f v1v2;   // v1.y, v1.z, v2.x, v2.y
	Vec4f v2norm; // v2.z, n.x, n.y, n.z
	Vec4f e1e2;   // e1.x, e1.y, e1.z, e2.x
	Vec4f e2matX; // e2.y, e2.z, mat_index, empty
};

struct MaterialGLSL
{
	Vec4f type_diffuse;  // mat_type, diff.x, diff.y, diff.z
	Vec4f specular_spec; // spec.x, spec.y, spec.z, n_spec
	Vec4f Le;			 // Le.x, Le.y, Le.z, empy
};

struct AABBGLSL
{
	Vec4f data1; // bmin.xyz, bmax.x
	Vec2f data2; // bmax.yz
};

struct BVHNodeGLSL
{
	AABBGLSL node_AABB;
	uint32 data[2]; // left/first_tri, num_tris
};

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;

	uint32 width = (uint32)WIDTH;
	uint32 height = (uint32)HEIGHT;
	// float aspect_atio = (float)width / (float)height;

	Display display = CreateDisplay("Pathtracer", width, height);
	InitRenderBuffer(display);

	// Memory allocation for bitmap buffer
	// Array<uint8> bitmap_buffer = CreateArray<uint8>(width * height * 3);

	// x is right, y is up, z is backwards
	// Vec3f eye = CreateVec3f(0.0f, 0.0f, 0.0f);

	// float grid_height = 2.0f;
	// float grid_width = aspect_atio * grid_height;
	// Vec3f grid_x = { grid_width, 0.0f, 0.0f };
	// Vec3f grid_y = { 0.0f, -grid_height,
					// 0.0f }; // negative because of the way I am writing to the file

	// Lower left corner of virtual grid
	// Vec3f grid_origin = eye - (grid_x / 2.0f) - (grid_y / 2.0f);
	// grid_origin.z = -2.0f;

	Array<Triangle> tris = CreateArray<Triangle>();
	Array<Material *> materials = CreateArray<Material *>();

	if (!LoadModelFromObj("CornellBox-Suzanne.obj", "../res/", tris, materials))
	{
		// DeallocateArray(bitmap_buffer);
		return -1;
	}

	printf("Loaded %lu triangles!\n", tris.size);

	// Apply model matrix to tris
	Mat4f model_matrix = CreateIdentityMat4f();

	// for cornell box
	// model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.0f, -3.5f), model_matrix); 

	// for robot
	model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.5f, -4.f), model_matrix);
	model_matrix = ScaleMat4f(CreateVec3f(0.2f, 0.2f, 0.2f), model_matrix);

	for (uint32 i = 0; i < tris.size; i++)
	{
		tris[i].v0 = model_matrix * tris[i].v0;
		tris[i].v1 = model_matrix * tris[i].v1;
		tris[i].v2 = model_matrix * tris[i].v2;
		tris[i].edge1 = tris[i].v1 - tris[i].v0;
		tris[i].edge2 = tris[i].v2 - tris[i].v0;
	}

	// Construct BVH tree and sort triangle list according to it
	Array<BVHNode> bvh_tree = CreateArray<BVHNode>(2 * tris.size - 1);
	
	BVHNode root_node {};
	root_node.first_tri = 0;
	AppendToArray(bvh_tree, root_node);
	
	if (!ConstructBVHSweepSAH(tris.data, tris.size, bvh_tree, 0))
	// if (!ConstructBVHObjectMedian(tris.data, tris.size, bvh_tree, 0))
	{
		printf("Error in BVH construction!\n");
		// DeallocateArray(bitmap_buffer);
		return -1;
	}
	printf("Finished building BVH!\n");
	printf("--- Number of BVH nodes: %lu\n", bvh_tree.size);

	// Find all emissive triangles in scene
	Array<uint32> emissive_tris = CreateArray<uint32>(tris.size);
	uint32 num_emissive_tris = 0;
	for (uint32 i = 0; i < tris.size; i++)
	{
		Material *current_mat = materials[tris[i].mat_index];
		if (current_mat->Le.x >= 0.01f || current_mat->Le.y >= 0.01f || current_mat->Le.z >= 0.01f)
		{
			AppendToArray(emissive_tris, i);
		}
	}

	Array<Sphere> spheres = CreateArray<Sphere>();
	Scene scene = ConstructScene(spheres, tris, emissive_tris, materials/*bvh_tree*/);

/*
	// Each thread's handle and data to be used by it
	void *threadHandles[NUM_THREADS];
	RenderData *data_for_threads[NUM_THREADS];

	// Variables that keep track of the chunk data
	uint32 pixel_step = 64;
	uint32 row_index = 0, col_index = 0;

	uint32 pixel_size = 3 * sizeof(uint8);
	uint32 row_size_in_bytes = pixel_size * width;
	uint32 thread_memory_size = pixel_step * pixel_step * pixel_size;

	uint16 num_chunk_columns = (uint16)Ceil((float)width / (float)pixel_step);
	uint16 num_chunk_rows = (uint16)Ceil((float)height / (float)pixel_step);

	// If there are more threads than needed, don't allocate memory for them
	uint16 num_threads = Min((uint16)NUM_THREADS, num_chunk_rows * num_chunk_columns);

	// Allocate memory for each one of the threads that will
	// be reused for all chunks to come
	for (uint16 i = 0; i < num_threads; i++)
	{
		uint8 *thread_memory = (uint8 *)malloc(thread_memory_size);

		data_for_threads[i] = (RenderData *)malloc(sizeof(RenderData));

		*(data_for_threads[i]) = { (void *)thread_memory,
								 thread_memory_size,
								 0,
								 0,
								 0,
								 0,
								 width,
								 height,
								 grid_origin,
								 grid_x,
								 grid_y,
								 eye,
								 scene,
								 false };
	}

	// Start a thread with the next chunk to be rendered
	for (uint16 i = 0; i <= num_threads; i++)
	{
		// Cycle around the thread array and check each thread infinitely
		if (i == num_threads)
			i = 0;

		// If we're at the end of the row, go down to the next row
		if (col_index == num_chunk_columns)
		{
			col_index = 0;
			row_index++;
		}

		// We rendered all rows
		if (row_index == num_chunk_rows)
		{
			break;
		}

		// We can start a thread if it has never been started, OR
		// if the thread has started and finished its execution
		RenderData *data = data_for_threads[i];
#if defined(_WIN32) || defined(_WIN64)
		if (!data->initialized || CanThreadStartWin32(threadHandles[i]))
#elif defined(__linux__)
		if (!data->initialized || CanThreadStartLinux(threadHandles[i]))
#endif
		{
			// If initialized, this means that there was a previous run
			// that we can save to the bitmap buffer. If not, we need
			// to initialize and start the thread for the first time
			if (data->initialized)
			{
				printf("Rendered region: x = %u-%u, y = %u-%u\n", data->start_x, data->end_x, data->start_y, data->end_y);

				int32 delta_x = (int32)(data->end_x - data->start_x);
				int32 delta_y = (int32)(data->end_y - data->start_y);

				// Read only this many bytes at a time from the thread's memory
				// and write it to the bitmap buffer. If we exceed this number
				// of bytes, we will enter the next row and write bytes to it
				uint32 chunk_row_size_in_bytes = pixel_size * delta_x;

				if (delta_x <= 0 || delta_y <= 0)
				{
					printf("Oh no we gotta go\n");
				}

				// Write the rendered region into the bitmap memory
				for (uint32 ymem = data->start_y; ymem < data->end_y; ymem++)
				{
					uint8 *offset_bitmap_buffer = bitmap_buffer.data + (ymem * row_size_in_bytes) + data->start_x * pixel_size;
					memcpy(offset_bitmap_buffer,
						   (uint8 *)data->thread_memory_chunk + (ymem - data->start_y) * chunk_row_size_in_bytes,
						   chunk_row_size_in_bytes);
				}

				// Close the finished thread because it can't be rerun
#if defined(_WIN32) || defined(_WIN64)
				CloseThreadWin32(threadHandles[i]);
#endif
			}

			// Set the thread's region parameters
			data->start_x = col_index * pixel_step;
			data->end_x = Min(width, data->start_x + pixel_step);
			data->start_y = row_index * pixel_step;
			data->end_y = Min(height, data->start_y + pixel_step);

			// Make sure next run's rendered region is saved
			data->initialized = true;

			// Create another thread with the above data being the same
#if defined(_WIN32) || defined(_WIN64)
			threadHandles[i] = CreateThreadWin32(data_for_threads[i]);
#elif defined(__linux__)
			threadHandles[i] = CreateThreadLinux(data_for_threads[i]);
#endif

			// Increment column index
			col_index++;
		}
	}

	// After we finish rendering all chunks
	for (uint16 i = 0; i < num_threads; i++)
	{
#if defined(_WIN32) || defined(_WIN64)
		// If the thread is currently running, wait for it (join it to main thread)
		if (!CanThreadStartWin32(threadHandles[i]))
			WaitForThreadWin32(threadHandles[i]);

		// Close the thread, we won't be needing it anymore
		CloseThreadWin32(threadHandles[i]);
#elif defined(__linux__)
		// If the thread is currently running, wait for it (join it to main thread)
		if (!CanThreadStartLinux(threadHandles[i]))
			WaitForThreadLinux(threadHandles[i]);
#endif

		// Thread just finished, so we need to write the changes
		RenderData *data = data_for_threads[i];

		printf("Rendered region: x = %u-%u, y = %u-%u\n", data->start_x, data->end_x,
			   data->start_y, data->end_y);

		uint32 chunk_row_size_in_bytes = pixel_size * (data->end_x - data->start_x);

		for (uint32 ymem = data->start_y; ymem < data->end_y; ymem++)
		{
			uint8 *offset_bitmap_buffer = bitmap_buffer.data + (ymem * row_size_in_bytes) + data->start_x * pixel_size;
			memcpy(offset_bitmap_buffer,
				   (uint8 *)data->thread_memory_chunk + (ymem - data->start_y) * chunk_row_size_in_bytes,
				   chunk_row_size_in_bytes);
		}

		// Free the allocated thread's memory, and the thread's data struct
		free(data->thread_memory_chunk);
		free(data);
	}

	// Write resulting rendered image to file
	FILE *result = fopen("../result.ppm", "w+");
	if (!result)
	{
		printf("Failed to create file!\n");
		DeallocateArray(bitmap_buffer);
		DeallocateArray(bvh_tree);
		return -1;
	}
	fprintf(result, "P3\n%d %d\n255\n", width, height);

	for (uint32 i = 0; i < (uint32)(width * height * 3); i += 3)
		fprintf(result, "%d %d %d\n", bitmap_buffer.data[i], bitmap_buffer.data[i + 1], bitmap_buffer.data[i + 2]);

	fclose(result);
	printf("Finished rendering to image!\n");
*/

	// Set up data to be passed to SSBOs

	Array<SphereGLSL> spheres_ssbo_data = CreateArray<SphereGLSL>();
	// spheres_ssbo_data.spheres[0] = { CreateVec4f(0.0f, 0.0f, -5.0f, 1.0f), {0} };
	// spheres_ssbo_data.spheres[1] = { CreateVec4f(0.0f, -101.0f, -5.0f, 100.0f), {0} };

	Array<TriangleGLSL> model_tris_ssbo_data = CreateArray<TriangleGLSL>(tris.size);
	for(uint32 i = 0; i < tris.size; i++)
	{
		const Triangle &current_tri = tris[i];
		const Vec3f &v0 = current_tri.v0;
		const Vec3f &v1 = current_tri.v1;
		const Vec3f &v2 = current_tri.v2;
		const Vec3f &e1 = current_tri.edge1;
		const Vec3f &e2 = current_tri.edge2;
		Vec3f n = NormalizeVec3f(Cross(e1, e2));

		TriangleGLSL tmp;
		tmp.v0v1 = CreateVec4f(v0.x, v0.y, v0.z, v1.x);
		tmp.v1v2 = CreateVec4f(v1.y, v1.z, v2.x, v2.y);
		tmp.v2norm = CreateVec4f(v2.z, n.x, n.y, n.z);
		tmp.e1e2 = CreateVec4f(e1.x, e1.y, e1.z, e2.x);
		tmp.e2matX = CreateVec4f(e2.y, e2.z, (float)current_tri.mat_index, 0.0f);

		AppendToArray(model_tris_ssbo_data, tmp);
	}

	Array<MaterialGLSL> materials_ssbo = CreateArray<MaterialGLSL>(materials.size);
	for(uint32 i = 0; i < materials.size; i++)
	{
		const Material *current_mat = materials[i];
		const Vec3f &diff = current_mat->diffuse;
		const Vec3f &spec = current_mat->specular;
		const Vec3f &Le = current_mat->Le;

		MaterialGLSL tmp;
		tmp.type_diffuse = CreateVec4f((float)current_mat->type, diff.x, diff.y, diff.z);
		tmp.specular_spec = CreateVec4f(spec.x, spec.y, spec.z, (float)current_mat->n_spec);
		tmp.Le = CreateVec4f(Le.x, Le.y, Le.z, 0.0f);

		AppendToArray(materials_ssbo, tmp);
	}

	Array<BVHNodeGLSL> bvh_ssbo = CreateArray<BVHNodeGLSL>(bvh_tree.size);
	for(uint32 i = 0; i < bvh_tree.size; i++)
	{
		const BVHNode &current_node = bvh_tree[i];
		const AABB &current_aabb = current_node.node_AABB;

		AABBGLSL aabb;
		aabb.data1 = CreateVec4f(current_aabb.bmin.x, current_aabb.bmin.y, current_aabb.bmin.z, current_aabb.bmax.x);
		aabb.data2 = CreateVec2f(current_aabb.bmax.y, current_aabb.bmax.z);

		BVHNodeGLSL tmp;
		tmp.node_AABB = aabb;
		tmp.data[0] = current_node.first_tri;
		tmp.data[1] = current_node.num_tris;

		AppendToArray(bvh_ssbo, tmp);
	}

	// Set up SSBOs
	GLuint ssbo[5];
	glGenBuffers(5, ssbo);

	// Sphere SSBO
	if(spheres_ssbo_data.size > 0)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, spheres_ssbo_data.size * sizeof(SphereGLSL), &(spheres_ssbo_data.data[0]), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[0]);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, model_tris_ssbo_data.size * sizeof(TriangleGLSL), &(model_tris_ssbo_data[0]), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[1]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, emissive_tris.size * sizeof(uint32), &(emissive_tris[0]), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[2]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, materials_ssbo.size * sizeof(MaterialGLSL), &(materials_ssbo[0]), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[3]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[4]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bvh_ssbo.size * sizeof(BVHNodeGLSL), &(bvh_ssbo[0]), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo[4]);

	glUseProgram(display.rb_shader_program);
	glBindTextureUnit(0, display.render_buffer_texture);
	glUniform1i(glGetUniformLocation(display.rb_shader_program, "tex"), 0);

	glUseProgram(display.compute_shader_program);
	glUniform1i(glGetUniformLocation(display.compute_shader_program, "screen"), 0);

	uint32 last_time = 0;
	uint32 last_report = 0;
	uint32 frame_count = 0;

	glUseProgram(display.compute_shader_program);
	uint32 u_seed_location = glGetUniformLocation(display.compute_shader_program, "u_seed");
	uint32 u_time_location = glGetUniformLocation(display.compute_shader_program, "u_time");
	uint32 frame_count_location = glGetUniformLocation(display.compute_shader_program, "frame_count");

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

		// Compute shader data and dispatch
		glUseProgram(display.compute_shader_program);

		// Update uniforms
		glUniform1ui(u_seed_location, pcg32_random());
		glUniform1ui(u_time_location, current_time);
		glUniform1ui(frame_count_location, frame_count++);

		glDispatchCompute(width / 8, height / 4, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Full screen quad drawing
		glUseProgram(display.rb_shader_program);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

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

	// DeallocateArray(bitmap_buffer);
	DeallocateArray(tris);
	for (uint32 i = 0; i < materials.size; i++)
	{
		free(materials[i]);
	}

	DeallocateArray(materials);
	// DeallocateArray(bvh_tree);

	CloseDisplay(display);

	return 0;
}