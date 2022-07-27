#include "display.hpp"
#include "../resource/shader.hpp"
#include <SDL.h>
#include <cstdio>
#include <glad/glad.h>

Display CreateDisplay(const char *title, uint32 width, uint32 height)
{
	Display result {};
	result.width = width;
	result.height = height;
	result.is_open = true;

	if (SDL_Init(SDL_INIT_VIDEO))
	{
		printf("ERROR: Failed to initialize SDL.\n");
		exit(-1);
	}

	// Load the default OGL library and get all function
	// definitions via GLAD down below.
	SDL_GL_LoadLibrary(nullptr);

	// OpenGL context version and profile
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Bit depth
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	// Window creation
	result.window_handle = SDL_CreateWindow(title,
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											(int32)width, (int32)height,
											SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (result.window_handle == nullptr)
	{
		printf("ERROR (WINDOW): Failed to create SDL Window!\n");
		exit(-1);
	}

	// Create OpenGL context and attach to window
	result.context = SDL_GL_CreateContext(result.window_handle);

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		printf("ERROR (WINDOW): Failed to initialize OpenGL context\n");
		exit(-1);
	}

	// Query limits
	GLint data1, data2, data3;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &data1);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &data2);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &data3);
	printf("Max size of work group (x, y, z): %d %d %d\n", data1, data2, data3);

	// Turn off VSync
	//SDL_GL_SetSwapInterval(0);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	return result;
}

bool InitRenderBuffer(Display &window)
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glViewport(0, 0, window.width, window.height);

	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	float uvs[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f
	};

	// Set up the fullscreen tris
	glGenVertexArrays(1, &window.vao);
	glBindVertexArray(window.vao);

	GLuint vbo[2];
	glCreateBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glNamedBufferStorage(vbo[0], sizeof(vertices), vertices, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glNamedBufferStorage(vbo[1], sizeof(uvs), uvs, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)nullptr);
	glEnableVertexAttribArray(1);

	// Set up the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &window.render_buffer_texture);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(window.render_buffer_texture, 1, GL_RGBA32F, window.width, window.height);
	glBindImageTexture(0, window.render_buffer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTextureUnit(0, window.render_buffer_texture);

	window.rb_shader_program = LoadShaderFromFiles("../../shaders/framebuffer.vert", "../../shaders/framebuffer.frag");
	window.compute_shader_program = LoadShaderFromFiles("../../shaders/framebuffer.comp");

	return true;
}

void CloseDisplay(Display &window)
{
	SDL_GL_DeleteContext(window.context);
	SDL_DestroyWindow(window.window_handle);
	SDL_Quit();
}

void UpdateDisplayTitle(Display &window, const char *new_title)
{
	SDL_SetWindowTitle(window.window_handle, new_title);
}
