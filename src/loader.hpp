#pragma once
#include "defines.hpp"

bool LoadModelFromObj(const char *path, const char *mltPath, 
					  struct Triangle **outTris, uint32 *numOutTris,
					  uint32 **outEmissiveTris, uint32 *numOutEmissiveTris,
					  struct Material **outMaterials, uint32 *numOutMaterials);