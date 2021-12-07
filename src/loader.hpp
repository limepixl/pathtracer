#pragma once
#include "defines.hpp"

bool LoadModelFromObj(const char *fileName, const char *path, 
					  struct Triangle **outTris, uint32 *numOutTris,
					  uint32 **outEmissiveTris, uint32 *numOutEmissiveTris,
					  struct Material **outMaterials, uint32 *numOutMaterials);