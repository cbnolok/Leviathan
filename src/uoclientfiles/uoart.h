#ifndef UOART_H
#define UOART_H

#include <string>
#include <vector>
#include <memory>
#include <QDateTime>

#include "uoidx.h"


class QImage;


namespace uopp
{
    class UOPError;
    class UOPPackage;
}


namespace uocf
{


class UOHues;


class UOArt
{  
public:
    UOArt(std::string clientPath, UOHues* hues = nullptr);
    ~UOArt();

    static const int kLandtilesOffset   = 0;
    static const int kItemsOffset       = 0x4000;   // the (world)items are stored after this id
    enum class ClientFileType
    {
        Uninitialized,
        ArtMUL,             // pre-Stygian Abyss (SA) art file
        ArtLegacyMulUOP,    // Classic Client (CC) art file
        LegacyTextureUOP,   // Enhanced Client art file, storing CC art tiles
        TextureUOP          // Enhanced Client art file, storing Kingdom Reborn (KR) art tiles
    };
private:
    static constexpr const char* kEC_UOPFile = "Texture.uop";
    static constexpr const char* kEC_LegacyUOPFile = "LegacyTexture.uop";
    static constexpr const char* kCC_UOPFile = "artLegacyMUL.uop";

public:
    void setHuesCachePointer(UOHues* hues);

    QImage* drawArt(unsigned int id, unsigned int hueIndex, bool partialHue);   // auto pick the newer art file format and draw the image
    QImage* drawArtEnhanced(bool drawLegacy, unsigned int id, unsigned int hueIndex, bool partialHue);
    QImage* drawArtClassic(bool drawFromUOP, unsigned int id, unsigned int hueIndex, bool partialHue);
    
private:
    void loadUOP(ClientFileType fileType, const QDateTime& lastModified, const std::string& uopPath, uopp::UOPError* uopError);
    bool getClassicPixelData(bool drawFromUOP, unsigned int id, std::vector<char> *data);

    std::string m_clientPath;
    ClientFileType m_lastFileType;
    QDateTime m_uopLastModified;
    std::unique_ptr<uopp::UOPPackage> m_uopPackage;

public:
    UOHues* m_UOHues;
};


}

#endif // UOART_H
