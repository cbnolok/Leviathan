#ifndef HELPERS_H
#define HELPERS_H


namespace uocf
{

void remap_BGRA32_to_RGBA32_image(int width, int length, unsigned char* decompressedImageData);
unsigned int remap_RGBA32_to_ARGB32_pixel(unsigned int RGBA);

}

#endif // HELPERS_H
