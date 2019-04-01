#include "colors.h"

namespace uocf
{

ARGB32 convert_ARGB16_to_ARGB32(const ARGB16 argb16, bool maxOpacity) noexcept
{
    // In ARGB16 each color is 5 bits, so a value between 0-31
    uint8_t a = uint8_t(argb16.getA());
    uint8_t r = uint8_t(argb16.getR());
    uint8_t g = uint8_t(argb16.getG());
    uint8_t b = uint8_t(argb16.getB());

    // In ARGB32 each color is 8 bits, so a value between 0-255
    // Convert them to 0-255 range:
    /*
    There is more than one way. You can just shift them left:
    to 00000000 rrrrr000 gggggg00 bbbbb000
    r <<= 3;
    g <<= 2;
    b <<= 3;
    But that means your image will be slightly dark and
    off-colour as white 0xFFFF will convert to F8,FC,F8
    So instead you can scale by multiply and divide:
*/
    // More exact conversion
    // This ensures 31/31 converts to 255/255
    //r = (uint8_t)(r * 255 / 31);        // R
    //g = (uint8_t)(g * 255 / 31);        // G
    //b = (uint8_t)(b * 255 / 31);        // B

    // Conversion from (AxisII)
    // Image slightly darker (probably though it looks more like the img shown in the uo client)
    r *= 8;        // R
    g *= 8;        // G
    b *= 8;        // B

    // If the pixel is black, set it to be full transparent
    if (maxOpacity)
        a = 255;
    //else if ( !r && !g && !b )
    //    a = 0;  // max transparency

    return ARGB32(a, r, g, b);
}

ARGB16 convert_ARGB32_to_ARGB16(const ARGB32 argb32, bool maxOpacity) noexcept
{
    unsigned char a = argb32.getA();
    unsigned char r = argb32.getR();
    unsigned char g = argb32.getG();
    unsigned char b = argb32.getB();

    r /= 8;        // R
    g /= 8;        // G
    b /= 8;        // B

    if (maxOpacity)
        a = 255;

    return ARGB16(a, r, g, b);
}


void ARGB32::adjustBrightness(int percent) noexcept
{
    int tempR = m_color_r + ((m_color_r * percent) / 100);
    if (tempR > 255)    tempR = 255;
    else if (tempR < 0) tempR = 0;
    m_color_r = uint8_t(tempR);

    int tempG = m_color_g + ((m_color_g * percent) / 100);
    if (tempG > 255)    tempG = 255;
    else if (tempG < 0) tempG = 0;
    m_color_g = uint8_t(tempG);

    int tempB = m_color_b + ((m_color_r * percent) / 100);
    if (tempB > 255)    tempB = 255;
    else if (tempB < 0) tempB = 0;
    m_color_b = uint8_t(tempB);
}

}
