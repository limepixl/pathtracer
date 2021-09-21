#pragma once
#include "defines.hpp"

bool LoadObjModel(const char *path, struct Triangle **outTris, int32 *numOutTris, int16 matIndex);