#pragma once
#include "defines.hpp"
#include "core/array.hpp"

bool LoadModelFromObj(const char *file_name, const char *path,
					  Array<struct Triangle> &out_tris,
					  Array<struct Material *> &out_materials);