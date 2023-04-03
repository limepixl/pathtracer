#pragma once
#include "../defines.hpp"
#include "glad/glad.h"

struct Shader
{
    GLuint id;
    bool initialized = false;

    Shader();
    explicit Shader(uint32 program_id);
};

Shader LoadShaderFromFiles(const char *vertex_source_path, const char *fragment_source_path);

Shader LoadShaderFromFiles(const char *compute_source_path);
