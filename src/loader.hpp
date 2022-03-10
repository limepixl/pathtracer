#pragma once
#include "defines.hpp"

bool LoadModelFromObj(const char *fileName, const char *path, 
					  struct Triangle **outTris, uint32 *numOutTris,
					  struct Material **outMaterials, uint32 *numOutMaterials);