#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>

namespace uocf
{

void remap_BGRA32_to_RGBA32_image(int width, int length, unsigned char* decompressedImageData);
uint32_t remap_RGBA32_to_ARGB32_pixel(uint32_t RGBA);

}

#endif // HELPERS_H
