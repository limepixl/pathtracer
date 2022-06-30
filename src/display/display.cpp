#include "display.hpp"
#include <glad/glad.h>
#include <SDL.h>
#include <cstdio>

Display CreateDisplay(const char *title, uint32 width, uint32 height)
{
	Display result {};
	result.width = width;
	result.height = height;
	result.is_open = true;

	if(SDL_Init(SDL_INIT_VIDEO))
	{
		printf("ERROR: Failed to initialize SDL.\n");
		exit(-1);
	}

	// Load the default OGL library and get all function
	// definitions via GLAD down below.
	SDL_GL_LoadLibrary(NULL);

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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Window creation
	result.window_handle = SDL_CreateWindow(title,
										    SDL_WINDOWPOS_CENTERED,
										    SDL_WINDOWPOS_CENTERED,
										    width, height,
										    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if(result.window_handle == nullptr)
	{
		printf("ERROR (WINDOW): Failed to create SDL Window!\n");
		exit(-1);
	}

	// Create OpenGL context and attach to window
	result.context = SDL_GL_CreateContext(result.window_handle);

	if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		printf("ERROR (WINDOW): Failed to initialize OpenGL context\n");
		exit(-1);
	}

	return result;
}

bool InitRenderBuffer(Display &window)
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glViewport(0, 0, window.width, window.height);

	float vertices[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	uint32 indices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	float uvs[] =
	{
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	// Set up the fullscreen tris
	glGenVertexArrays(1, &window.vao);
	glBindVertexArray(window.vao);

	GLuint vbo[2];
	glGenBuffers(2, vbo);
	glGenBuffers(1, &window.ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set up the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &window.render_buffer_texture);
	glBindTexture(GL_TEXTURE_2D, window.render_buffer_texture);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(window.render_buffer_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(window.render_buffer_texture, 1, GL_RGBA32F, window.width, window.height);
	glBindImageTexture(0, window.render_buffer_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Set up render buffer shader
	const char *vertex_shader_source = "#version 460 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec2 uvs;\n"
	"out vec2 frag_uvs;\n"
	"void main() {\n"
	"gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
	"frag_uvs = uvs;\n}";

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	GLint compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(vertex_shader, 1024, &log_length, message);
		printf("SHADER COMPILATION ERROR:\n%s\n", message);
	}

	const char *fragment_shader_source = "#version 460 core\n"
	"in vec2 frag_uvs;\n"
	"out vec4 color;\n"
	"uniform sampler2D tex;\n"
	"void main() { color = texture(tex, frag_uvs); }\n";

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(fragment_shader, 1024, &log_length, message);
		printf("SHADER COMPILATION ERROR:\n%s\n", message);
	}

	window.rb_shader_program = glCreateProgram();
	glAttachShader(window.rb_shader_program, vertex_shader);
	glAttachShader(window.rb_shader_program, fragment_shader);
	glLinkProgram(window.rb_shader_program);

	GLint is_linked;
	glGetProgramiv(window.rb_shader_program, GL_LINK_STATUS, &is_linked);
	if(is_linked == GL_FALSE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(window.rb_shader_program, 1024, &log_length, message);
		printf("SHADER PROGRAM LINKING ERROR:\n%s\n", message);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Set up compute shader
	// Set up the compute shader
	const char *compute_shader_source = "#version 460 core\n"
	"layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;\n"
	"layout(rgba32f, binding = 0) uniform image2D screen;\n"
	"void main() {\n"
	"ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);\n"
	"imageStore(screen, pixel_coords, vec4(float(gl_WorkGroupID.x) / gl_NumWorkGroups.x, float(gl_WorkGroupID.y) / gl_NumWorkGroups.y, 0.0, 1.0));}";

	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute_shader, 1, &compute_shader_source, NULL);
	glCompileShader(compute_shader);

	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(compute_shader, 1024, &log_length, message);
		printf("SHADER COMPILATION ERROR:\n%s\n", message);
	}

	window.compute_shader_program = glCreateProgram();
	glAttachShader(window.compute_shader_program, compute_shader);
	glLinkProgram(window.compute_shader_program);

	glGetProgramiv(window.compute_shader_program, GL_LINK_STATUS, &is_linked);
	if(is_linked == GL_FALSE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(window.compute_shader_program, 1024, &log_length, message);
		printf("SHADER PROGRAM LINKING ERROR:\n%s\n", message);
	}

	return true;
}

void CloseDisplay(Display &window)
{
	SDL_GL_DeleteContext(window.context);
	SDL_DestroyWindow(window.window_handle);
	SDL_Quit();
}