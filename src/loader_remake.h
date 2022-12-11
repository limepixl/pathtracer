#pragma once
#include "core/array.hpp"

bool LoadGLTF(const char *path, Array<struct Triangle> &out_tris, Array<struct Material> &out_mats);
