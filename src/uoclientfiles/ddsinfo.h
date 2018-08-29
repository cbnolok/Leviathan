#ifndef DDSINFO_H
#define DDSINFO_H

#include <string>

struct DDSInfo
{    
    DDSInfo(const char* DDSData);

    enum class TextureFormat
    {
        DXT1,
        DXT5,
        UnknownCompressed,
        RGBA32,
        RGB24,
        RGB565,
        ARGB4444,
        RGBA4444,
        Alpha8,
        Unknown
    };

    static constexpr const unsigned int kImageDataStartOffset = 4+124; // 4=magic, 124=DDS_HEADER

    std::string errorString;
    int height, width;
    TextureFormat textureFormat;

};

#endif // DDSINFO_H
