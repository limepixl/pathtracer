#pragma once
#include "defines.hpp"
#include "core/array.hpp"

bool LoadModelFromObj(const char *fileName, const char *path,
					  Array<struct Triangle> &outTris,
					  Array<struct Material *> &outMaterials);