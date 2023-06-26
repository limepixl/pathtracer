#include "shader.hpp"
#include "../core/array.hpp"

#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>

Shader::Shader(uint32 program_id)
    : id(program_id), initialized(true)
{}

Shader::Shader()
    : id(0), initialized(false)
{}

Shader LoadShaderFromFiles(const char *compute_source_path, bool isSpirV)
{
    if (compute_source_path == nullptr)
    {
        printf("ERROR (SHADER): Can't initialize shader program with no shader source!\n");
        exit(-1);
    }

    // Load shader from file

    FILE *compute_file = fopen(compute_source_path, "rb");
    if (compute_file == nullptr)
    {
        printf("ERROR (SHADER): Failed to load shader file at path: %s\n", compute_source_path);
        exit(-1);
    }

    fseek(compute_file, 0, SEEK_END);
	int64 file_length = ftell(compute_file);
    fseek(compute_file, 0, SEEK_SET);

    if (file_length <= 0)
    {
        printf("ERROR (SHADER): File length is wrong!\n");
        exit(-1);
    }

    Array<char> shader_source((uint64) file_length + 1);
    shader_source.size = (uint64) file_length + 1;
    fread(shader_source._data, (uint32) file_length * sizeof(char), 1, compute_file);
    shader_source[(uint32) file_length] = '\0';

    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	if (isSpirV)
	{
		glShaderBinary(1, &compute_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, shader_source._data, file_length);
		glSpecializeShader(compute_shader, "main", 0, nullptr, nullptr);
	}
	else
	{
    	glShaderSource(compute_shader, 1, &(shader_source._data), nullptr);
    	glCompileShader(compute_shader);
	}

    GLint buffer_length = 0;
    glGetShaderiv(compute_shader, GL_INFO_LOG_LENGTH, &buffer_length);
    if (buffer_length > 1)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(compute_shader, buffer_length, &log_length, message);
        printf("SHADER WARNING:\n%s\n", message);
    }

    GLint compiled = 0;
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(compute_shader, 1024, &log_length, message);
        printf("SHADER COMPILATION ERROR:\n%s\n", message);
        exit(-1);
    }

    fclose(compute_file);

    // Create program with loaded shader
    uint32 program = glCreateProgram();
    glAttachShader(program, compute_shader);
    glLinkProgram(program);

    GLint is_linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        printf("ERROR (SHADER): Shader program linking error!:\n%s\n", message);
        exit(-1);
    }

	glDetachShader(program, compute_shader);
    glDeleteShader(compute_shader);

    return Shader(program);
}

Shader LoadShaderFromFiles(const char *vertex_source_path,
                           const char *fragment_source_path)
{
    if (vertex_source_path == nullptr || fragment_source_path == nullptr)
    {
        printf("ERROR (SHADER): Can't initialize shader program with no shader source!\n");
        exit(-1);
    }

    // Load shaders from files

    FILE *vertex_file = fopen(vertex_source_path, "rb");
    if (vertex_file == nullptr)
    {
        printf("ERROR (SHADER): Failed to load shader file at path: %s\n", vertex_source_path);
        exit(-1);
    }

    fseek(vertex_file, 0, SEEK_END);
	int64 file_length = ftell(vertex_file);
    fseek(vertex_file, 0, SEEK_SET);

    if (file_length <= 0)
    {
        printf("ERROR (SHADER): File length is wrong!\n");
        exit(-1);
    }

    Array<char> shader_source((uint64) file_length + 1);
    shader_source.size = (uint64) file_length + 1;
    fread(shader_source._data, (uint32) file_length * sizeof(char), 1, vertex_file);
    shader_source[(uint32) file_length] = '\0';

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &(shader_source._data), nullptr);
    glCompileShader(vertex_shader);

    GLint compiled = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertex_shader, 1024, &log_length, message);
        printf("SHADER COMPILATION ERROR:\n%s\n", message);
    }

    fclose(vertex_file);

    FILE *fragment_file = fopen(fragment_source_path, "rb");
    if (fragment_file == nullptr)
    {
        printf("ERROR (SHADER): Failed to load shader file at path: %s\n", fragment_source_path);
        exit(-1);
    }

    fseek(fragment_file, 0, SEEK_END);
    file_length = ftell(fragment_file);
    fseek(fragment_file, 0, SEEK_SET);

    if (file_length <= 0)
    {
        printf("ERROR (SHADER): File length is wrong!\n");
        exit(-1);
    }

    shader_source = Array<char>((uint64) file_length + 1);
    shader_source.size = (uint64) file_length + 1;
    fread(shader_source._data, (uint32) file_length * sizeof(char), 1, fragment_file);
    shader_source[(uint32) file_length] = '\0';

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &(shader_source._data), nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragment_shader, 1024, &log_length, message);
        printf("SHADER COMPILATION ERROR:\n%s\n", message);
    }

    fclose(fragment_file);

    // Create program with loaded shaders
    uint32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint is_linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        printf("ERROR (SHADER): Shader program linking error!:\n%s\n", message);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return Shader(program);
}
