#include "ddsinfo.h"
#include <cstddef>
#include <cstring>

// Adapted from: https://github.com/sarbian/DDSLoader/blob/master/DatabaseLoaderTexture_DDS.cs

DDSInfo::DDSInfo(const char *DDSData) :
    height(0), width(0), textureFormat(TextureFormat::Unknown)
{
    using uint = unsigned int;
    static const uint DDSD_MIPMAPCOUNT_BIT  = 0x00020000;
    static const uint DDPF_ALPHAPIXELS      = 0x00000001;
    static const uint DDPF_ALPHA            = 0x00000002;
    static const uint DDPF_FOURCC           = 0x00000004;
    static const uint DDPF_RGB              = 0x00000040;
    //static const uint DDPF_YUV            = 0x00000200;
    static const uint DDPF_LUMINANCE        = 0x00020000;
    //static const uint DDPF_NORMAL         = 0x80000000;
    /*
    char[4] magic = "DDS ";
    struct DDS_HEADER {
      DWORD           dwSize;
      DWORD           dwFlags;
      DWORD           dwHeight;
      DWORD           dwWidth;
      DWORD           dwPitchOrLinearSize;
      DWORD           dwDepth;
      DWORD           dwMipMapCount;
      DWORD           dwReserved1[11];
      DDS_PIXELFORMAT ddspf;
      DWORD           dwCaps;
      DWORD           dwCaps2;
      DWORD           dwCaps3;
      DWORD           dwCaps4;
      DWORD           dwReserved2;
    };
    sizeof(DDS_HEADER) = 124
    */

    size_t dataOffset = 0;
#define READMEMFULL(dest, size) \
    memcpy(static_cast<void*>(&(dest)), static_cast<const void*>(DDSData+dataOffset), (size)); \
    dataOffset += (size);

    char magic[4]; READMEMFULL(magic, 4);
    if (strncmp((char*)magic, "DDS ", 4) != 0)
    {
        errorString = "Not a valid DDS";
        return;
    }

    //this header byte should be 124 for DDS image files
    uint dwSize = 0; READMEMFULL(dwSize, 4);
    if (dwSize != 124)
    {
        errorString = "Invalid header size";
        return;
    }

    uint dwFlags = 0; READMEMFULL(dwFlags, 4);
    READMEMFULL(height, 4);
    READMEMFULL(width, 4);

    uint dwPitchOrLinearSize = 0; READMEMFULL(dwPitchOrLinearSize, 4);
    uint dwDepth = 0; READMEMFULL(dwDepth, 4);
    uint dwMipMapCount = 0; READMEMFULL(dwMipMapCount, 4);

    if ((dwFlags & DDSD_MIPMAPCOUNT_BIT) == 0)
    {
        dwMipMapCount = 1;
    }

    // dwReserved1
    dataOffset += (4 * 11);

    // DDS_PIXELFORMAT
    uint dds_pxlf_dwSize = 0; READMEMFULL(dds_pxlf_dwSize, 4);
    uint dds_pxlf_dwFlags = 0; READMEMFULL(dds_pxlf_dwFlags, 4);
    char dds_pxlf_dwFourCC[4]; READMEMFULL(dds_pxlf_dwFourCC, 4);
    uint dds_pxlf_dwRGBBitCount = 0; READMEMFULL(dds_pxlf_dwRGBBitCount, 4);
    //uint pixelSize = dds_pxlf_dwRGBBitCount / 8;
    uint dds_pxlf_dwRBitMask = 0; READMEMFULL(dds_pxlf_dwRBitMask, 4);
    uint dds_pxlf_dwGBitMask = 0; READMEMFULL(dds_pxlf_dwGBitMask, 4);
    uint dds_pxlf_dwBBitMask = 0; READMEMFULL(dds_pxlf_dwBBitMask, 4);
    uint dds_pxlf_dwABitMask = 0; READMEMFULL(dds_pxlf_dwABitMask, 4);

    uint dwCaps = 0; READMEMFULL(dwCaps, 4);
    uint dwCaps2 = 0; READMEMFULL(dwCaps2, 4);
    uint dwCaps3 = 0; READMEMFULL(dwCaps3, 4);
    uint dwCaps4 = 0; READMEMFULL(dwCaps4, 4);
    uint dwReserved2 = 0; READMEMFULL(dwReserved2, 4);

    //bool isCompressed = false;
    //bool asNormal = false;
    //bool isNormalMap = (dds_pxlf_dwFlags & DDPF_NORMAL) != 0; //|| asNormal;

    bool alpha =        (dds_pxlf_dwFlags & DDPF_ALPHA) != 0;
    bool fourcc =       (dds_pxlf_dwFlags & DDPF_FOURCC) != 0;
    bool rgb =          (dds_pxlf_dwFlags & DDPF_RGB) != 0;
    bool alphapixel =   (dds_pxlf_dwFlags & DDPF_ALPHAPIXELS) != 0;
    bool luminance =    (dds_pxlf_dwFlags & DDPF_LUMINANCE) != 0;
    bool rgb888 =   dds_pxlf_dwRBitMask == 0x000000ff && dds_pxlf_dwGBitMask == 0x0000ff00 && dds_pxlf_dwBBitMask == 0x00ff0000;
    bool bgr888 =   dds_pxlf_dwRBitMask == 0x00ff0000 && dds_pxlf_dwGBitMask == 0x0000ff00 && dds_pxlf_dwBBitMask == 0x000000ff;
    bool rgb565 =   dds_pxlf_dwRBitMask == 0x0000F800 && dds_pxlf_dwGBitMask == 0x000007E0 && dds_pxlf_dwBBitMask == 0x0000001F;
    bool argb4444 = dds_pxlf_dwABitMask == 0x0000f000 && dds_pxlf_dwRBitMask == 0x00000f00 && dds_pxlf_dwGBitMask == 0x000000f0 && dds_pxlf_dwBBitMask == 0x0000000f;
    bool rbga4444 = dds_pxlf_dwABitMask == 0x0000000f && dds_pxlf_dwRBitMask == 0x0000f000 && dds_pxlf_dwGBitMask == 0x000000f0 && dds_pxlf_dwBBitMask == 0x00000f00;

    if (fourcc)
    {
        // Texture dos not contain RGB data, check FourCC for format
        //isCompressed = true;

        if (strncmp((char*)dds_pxlf_dwFourCC, "DXT1", 4) == 0)
            textureFormat = TextureFormat::DXT1;
        else if (strncmp((char*)dds_pxlf_dwFourCC, "DXT5", 4) == 0)
            textureFormat = TextureFormat::DXT5;
        else
        {
            textureFormat = TextureFormat::UnknownCompressed;
            errorString = "Compressed but not DXT1 nor DXT5";
        }
        return;
    }
    if (rgb && (rgb888 || bgr888)) // RGB or RGBA format
        textureFormat = alphapixel ? TextureFormat::RGBA32 : TextureFormat::RGB24;
    else if (rgb && rgb565) // Nvidia texconv B5G6R5_UNORM
        textureFormat = TextureFormat::RGB565;
    else if (rgb && alphapixel && argb4444) // Nvidia texconv B4G4R4A4_UNORM
        textureFormat = TextureFormat::ARGB4444;
    else if (rgb && alphapixel && rbga4444)
        textureFormat = TextureFormat::RGBA4444;
    else if (!rgb && alpha != luminance) // A8 format or Luminance 8
        textureFormat = TextureFormat::Alpha8;
    else
        textureFormat = TextureFormat::Unknown;
    errorString = "Not DXT1 nor DXT5";
}
