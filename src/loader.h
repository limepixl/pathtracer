#pragma once
#include "defines.hpp"
#include "core/array.hpp"

bool LoadGLTF(const char *path,
			  Array<struct Triangle> &out_tris,
			  Array<struct MaterialGLSL> &out_mats,
			  struct Mat4f &model_matrix,
			  uint32 &texture_array);
