#pragma once
#include "../defines.hpp"

uint32 LoadShaderFromFiles(const char *vertex_source_path, const char *fragment_source_path);
uint32 LoadShaderFromFiles(const char *compute_source_path);