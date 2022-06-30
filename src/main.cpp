#include "loader.hpp"
#include "scene/bvh.hpp"
#include "scene/material.hpp"
#include "scene/sphere.hpp"
#include "scene/triangle.hpp"
#include "threads.hpp"

#include <cstdlib>
#include <cstring>

#include "display/display.hpp"
#include <glad/glad.h>
#include <SDL.h>

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;

	uint32 width = (uint32)WIDTH;
	uint32 height = (uint32)HEIGHT;
	float aspect_atio = (float)width / (float)height;

	Display display = CreateDisplay("Pathtracer", width, height);
	InitRenderBuffer(display);

	// Memory allocation for bitmap buffer
	Array<uint8> bitmap_buffer = CreateArray<uint8>(width * height * 3);

	// x is right, y is up, z is backwards
	Vec3f eye = CreateVec3f(0.0f, 0.0f, 0.0f);

	float grid_height = 2.0f;
	float grid_width = aspect_atio * grid_height;
	Vec3f grid_x = { grid_width, 0.0f, 0.0f };
	Vec3f grid_y = { 0.0f, -grid_height,
					0.0f }; // negative because of the way I am writing to the file

	// Lower left corner of virtual grid
	Vec3f grid_origin = eye - (grid_x / 2.0f) - (grid_y / 2.0f);
	grid_origin.z = -2.0f;

	Array<Triangle> tris = CreateArray<Triangle>();
	Array<Material *> materials = CreateArray<Material *>();

	if (!LoadModelFromObj("CornellBox-Suzanne.obj", "../res/", tris, materials))
	{
		DeallocateArray(bitmap_buffer);
		return -1;
	}

	// Apply model matrix to tris
	Mat4f model_matrix = CreateIdentityMat4f();

	// for cornell box
	model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.0f, -3.5f), model_matrix); 

	// for robot
	// model_matrix = TranslationMat4f(CreateVec3f(0.0f, -1.5f, -4.f), model_matrix);
	// model_matrix = ScaleMat4f(CreateVec3f(0.2f, 0.2f, 0.2f), model_matrix);

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
		DeallocateArray(bitmap_buffer);
		return -1;
	}
	printf("Finished building BVH!\n");

	// Find all emissive triangles in scene
	Array<uint32> emissive_tris = CreateArray<uint32>(tris.size);
	uint32 num_emissive_tris = 0;
	for (uint32 i = 0; i < tris.size; i++)
	{
		if (tris[i].mat->Le.x >= 0.01f || tris[i].mat->Le.y >= 0.01f || tris[i].mat->Le.z >= 0.01f)
		{
			AppendToArray(emissive_tris, i);
		}
	}

	Array<Sphere> spheres = CreateArray<Sphere>();
	Scene scene = ConstructScene(spheres, tris, emissive_tris, bvh_tree);

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

	glUseProgram(display.rb_shader_program);
	glBindTextureUnit(0, display.render_buffer_texture);
	glUniform1i(glGetUniformLocation(display.rb_shader_program, "tex"), 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, bitmap_buffer.data);

	glUseProgram(display.compute_shader_program);
	glUniform1i(glGetUniformLocation(display.compute_shader_program, "screen"), 0);

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

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(display.compute_shader_program);
		glDispatchCompute(width / 8, height / 4, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glUseProgram(display.rb_shader_program);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

		SDL_GL_SwapWindow(display.window_handle);
	}

	DeallocateArray(bitmap_buffer);
	DeallocateArray(tris);
	for (uint32 i = 0; i < materials.size; i++)
	{
		free(materials[i]);
	}

	DeallocateArray(materials);
	DeallocateArray(bvh_tree);

	CloseDisplay(display);

	return 0;
}