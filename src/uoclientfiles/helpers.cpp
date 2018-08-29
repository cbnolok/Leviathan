#include "helpers.h"

namespace uocf
{

//For some compressions (not DTX5) Libsquish returns BGRA values, so we may need to change it to RGBA values
void remap_BGRA32_to_RGBA32_image(int width, int length, unsigned char* decompressedImageData)
{
    // Reworked from: https://github.com/merfed/Coffee/blob/master/Tools/BLP/Blpc/1.1/MDS.cBlp2/Blp2.cs
    for (int i = 0; i < width*length*4; i += 4)
    {
        decompressedImageData[i] ^= decompressedImageData[i + 2];
        decompressedImageData[i + 2] ^= decompressedImageData[i];
        decompressedImageData[i] ^= decompressedImageData[i + 2];
    }
}

uint32_t remap_RGBA32_to_ARGB32_pixel(uint32_t RGBA)
{
    uint8_t r = (uint8_t)(RGBA & 0xFF000000);
    uint8_t g = (uint8_t)(RGBA & 0x00FF0000);
    uint8_t b = (uint8_t)(RGBA & 0x0000FF00);
    uint8_t a = (uint8_t)(RGBA & 0x000000FF);
    return ((a << 24) | (r << 16) | (g << 8) | b);
}


}
