#define STB_DXT_IMPLEMENTATION
#include <Lib/STB/stb_dxt.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <Lib/STB/stb_image_resize.h>

#define STB_IMAGE_IMPLEMENTATION
// #define STBI_MALLOC(sz) ((void*)(new char[sz]))
// #define STBI_REALLOC(p,newsz) realloc(p,newsz)
// #define STBI_FREE(p) (delete[] ((char*)p))
#include <Lib/STB/stb_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
