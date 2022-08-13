#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "stb_image.h"
#pragma clang diagnostic pop
#else
#include "stb_image.h"
#endif
