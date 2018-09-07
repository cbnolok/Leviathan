#include "uoart.h"

#include <fstream>
#include <QFileInfo>
#include <QImage>

#include "../globals.h"
#include "../cpputils/sysio.h"
#include "../uoppackage/uoppackage.h"
#include "../uoppackage/uopfile.h"
#include "libsquish/squish.h"
#include "ddsinfo.h"
#include "uohues.h"


namespace uocf
{

UOArt::UOArt(std::string clientPath, UOHues *hues) :
    m_clientPath(std::move(clientPath)), m_lastFileType(ClientFileType::Uninitialized), m_UOHues(hues)
{
}

UOArt::~UOArt() = default;

void UOArt::setHuesCachePointer(UOHues* hues)
{
    m_UOHues = hues;
}

QImage* UOArt::drawArt(unsigned int id, unsigned int hueIndex, bool partialHue)
{
    std::string EC_UOPPath = m_clientPath + kEC_UOPFile;
    if (isValidFile(EC_UOPPath))
        return drawArtEnhanced(false, id, hueIndex, partialHue);

    std::string EC_LegacyUOPPath = m_clientPath + kEC_LegacyUOPFile;
    if (isValidFile(EC_LegacyUOPPath))
        return drawArtEnhanced(true, id, hueIndex, partialHue);

    std::string CC_UOPPath = m_clientPath + kCC_UOPFile;
    if (isValidFile(CC_UOPPath))
        return drawArtClassic(true, id, hueIndex, partialHue);

    return drawArtClassic(false, id, hueIndex, partialHue);
}

void UOArt::loadUOP(ClientFileType fileType, const QDateTime &lastModified, const std::string& uopPath, uopp::UOPError *uopError)
{
    m_lastFileType = fileType;
    m_uopLastModified = lastModified;
    m_uopPackage = std::make_unique<uopp::UOPPackage>();
    m_uopPackage->load(uopPath, uopError);
}

QImage* UOArt::drawArtEnhanced(bool drawLegacy, unsigned int id, unsigned int hueIndex, bool partialHue)
{
    // drawLegacy == false means that it's an enhanced client texture
    // drawLegacy == true means that's a classic client art, encoded as a dds texture for the enhanced client

    // Currently, only item arts are supported for the enhanced client. Legacy terrain arts will be supported in the future.
    if (id >= kItemsOffset)
        id -= kItemsOffset;

    uopp::UOPError uopError;
    uopp::UOPFile* uopFile = nullptr;
    if (!drawLegacy)
    {
        // Try to retrieve the TextureUOP image. If the image is missing, load it from the LegacyTextureUOP file
        std::string uopPath_EnhancedTexture = m_clientPath + kEC_UOPFile;
        QDateTime lastModified = QFileInfo(uopPath_EnhancedTexture.c_str()).lastModified();
        if ((m_lastFileType != ClientFileType::TextureUOP) || (lastModified != m_uopLastModified))
            loadUOP(ClientFileType::TextureUOP, lastModified, uopPath_EnhancedTexture, &uopError);

        std::string packedFileName_EnhancedTexture = QString("build/worldart/%1.dds").arg(id, 8, 10, QChar('0')).toStdString();
        uopFile = m_uopPackage->getFileByName(packedFileName_EnhancedTexture);
        if (uopFile == nullptr)
        {
            // Not found from TextureUOP. Actually there's not a 1:1 correspondence between CC and EC art texture IDs,
            //  because two ids can refer to a different piece of the same texture (like the ankh one).
            // So, TODO: parse TileArt.uop and get the right texture ids (and offsets) to show.
            drawLegacy = true;
        }
    }
    if (drawLegacy)
    {
        std::string uopPath_LegacyTexture = m_clientPath + kEC_LegacyUOPFile;
        QDateTime lastModified = QFileInfo(uopPath_LegacyTexture.c_str()).lastModified();
        if ((m_lastFileType != ClientFileType::LegacyTextureUOP) || (lastModified != m_uopLastModified))
            loadUOP(ClientFileType::LegacyTextureUOP, lastModified, uopPath_LegacyTexture, &uopError);

        std::string packedFileName_LegacyTexture = QString("build/tileartlegacy/%1.dds").arg(id, 8, 10, QChar('0')).toStdString();
        uopFile = m_uopPackage->getFileByName(packedFileName_LegacyTexture);
    }

    if (uopFile == nullptr)
    {
        appendToLog(QString("Error looking up %1 (requested id %2).").arg( (m_lastFileType == ClientFileType::TextureUOP ? kEC_UOPFile : kCC_UOPFile), id ).toStdString());
        appendToLog(uopError.buildErrorsString());
        return nullptr;
    }
    std::ifstream fin = m_uopPackage->getOpenedStream();
    if (fin.bad())
    {
        appendToLog(QString("Error seeking from %1 (requested id %2).").arg( (m_lastFileType == ClientFileType::TextureUOP ? kEC_UOPFile : kCC_UOPFile), id ).toStdString());
        appendToLog(uopError.buildErrorsString());
        return nullptr;
    }

    auto* decompressedDataVec = new std::vector<char>();
    if (!uopFile->readData(fin, &uopError) || !uopFile->unpack(decompressedDataVec, &uopError))
    {
        appendToLog(QString("Error unpacking from %1 (requested id %2).").arg( (m_lastFileType == ClientFileType::TextureUOP ? kEC_UOPFile : kCC_UOPFile), id ).toStdString());
        appendToLog(uopError.buildErrorsString());
        return nullptr;
    }

    const char* DDSDataPtr = decompressedDataVec->data();
    DDSInfo texInfo(DDSDataPtr);
    if (!texInfo.errorString.empty())
    {
        appendToLog(std::string("DDSInfo error: ") + texInfo.errorString);
        return nullptr;
    }

    // Decompress the DXT1(BC1)/DXT5(BC3) DDS image into plain RGBA32 (QImage::Format_RGBA8888) pixel data.
    // RGBA8888 will be returned independently from the compression of the source image (the concern of a different format
    //  was motivated by the fact that DXT1 doesn't preserve the alpha channel, so it actually has RGB888 data).
    auto* decompressedImgBuf = new unsigned char[texInfo.height * texInfo.width * 4]; // I don't know the real max size, so let's just be generous.
    squish::DecompressImage(decompressedImgBuf, texInfo.width, texInfo.height,
                            (void const*)(DDSDataPtr + DDSInfo::kImageDataStartOffset),
                            (texInfo.textureFormat == DDSInfo::TextureFormat::DXT1) ? squish::kDxt1 : squish:: kDxt5);

    // Load pixel data into a QImage
    //  The const cast for decompressedImgBuf is important! From Qt documentation:
    // "Unlike the similar QImage constructor that takes a non-const data buffer, this version will never alter the
    // contents of the buffer. For example, calling QImage::bits() will return a deep copy of the image, rather than
    // the buffer passed to the constructor."
    QImage imageBuf = QImage((const uchar*)decompressedImgBuf, texInfo.width, texInfo.height, QImage::Format_RGBA8888);
    QImage* image = new QImage();
    *image = imageBuf.copy();
    // I exploit this behavior of the const-variant QImage constructor to perform a deep copy to our definitive QImage (image),
    //  from the QImage (imageBuf) that used as internal buffer the pointer to our decompressedImgBuf.
    // By doing so, our QImage will be independent from our buffer, so we can finally delete it.
    // Without doing this, we'll encounter random crashes when the image is used.
    delete decompressedImgBuf;

    // Apply hue
    if (hueIndex)
    {
        for (int x = 0; x < texInfo.width; ++x)
        {
            for (int y = 0; y < texInfo.height; ++y)
            {
                // QImage::pixel returns a QRgb, which is ARGB quadruplet on the format #AARRGGBB, equivalent to an unsigned int.
                unsigned int argb32 = image->pixel(x, y);
                ARGB16 argb16 = convert_ARGB32_to_ARGB16(ARGB32(argb32));
                argb16 = m_UOHues->getHueEntry(hueIndex).applyToColor(argb16, partialHue);
                argb32 = convert_ARGB16_to_ARGB32(argb16).getVal();
                image->setPixel(x, y, argb32);
            }
        }
    }

    return image;
}

bool UOArt::getClassicPixelData(bool drawFromUOP, unsigned int id, std::vector<char> *data)
{
    if (drawFromUOP)
    {
        // UOP
        std::string CC_UOPPath = m_clientPath + kCC_UOPFile;
        QDateTime lastModified = QFileInfo(CC_UOPPath.c_str()).lastModified();
        uopp::UOPError uopError;
        if (lastModified != m_uopLastModified)
        {
            loadUOP(ClientFileType::TextureUOP, lastModified, CC_UOPPath, &uopError);
            if (uopError.errorOccurred())
            {
                appendToLog(uopError.buildErrorsString());
                return false;
            }
        }

        std::string packedFileName = QString("build/artlegacymul/%1.tga").arg(id, 8, 10, QChar('0')).toStdString();
        uopp::UOPFile* uopFile = m_uopPackage->getFileByName(packedFileName);
        if (uopFile == nullptr)
        {
            appendToLog(QString("Error looking up %1 (requested id %2).").arg(kCC_UOPFile, id).toStdString());
            return false;
        }
        std::ifstream fin = m_uopPackage->getOpenedStream();
        if (fin.bad())
        {
            appendToLog(QString("Error seeking from %1 (requested id %2).").arg(kCC_UOPFile, id).toStdString());
            return false;
        }

        auto* decompressedDataVec = new std::vector<char>();
        if (!uopFile->readData(fin, &uopError) || !uopFile->unpack(decompressedDataVec, &uopError))
        {
            appendToLog(QString("Error unpacking from %1 (requested id %2).").arg(kCC_UOPFile, id).toStdString());
            appendToLog(uopError.buildErrorsString());
            return false;
        }
        *data = *uopFile->getDataVec();
    }
    else
    {
        // IDX
        std::string IDXPath = m_clientPath + "artidx.mul";
        UOIdx::Entry idxEntry = {};
        if (! UOIdx::getLookup(IDXPath, id, &idxEntry))
        {
            appendToLog(QString("Error looking up artidx.mul (requested id %1).").arg(id).toStdString());
            return false;
        }
        std::ifstream fin;
        fin.open(m_clientPath + "art.mul", std::ifstream::in | std::ifstream::binary);
        if (!fin.is_open())
        {
            appendToLog("Error loading art.mul");
            return false;
        }
        fin.seekg(idxEntry.lookup);

        data->resize(idxEntry.size);
        fin.read(data->data(), idxEntry.size);
        fin.close();
    }
    return true;
}

QImage* UOArt::drawArtClassic(bool drawFromUOP, unsigned int id, unsigned int hueIndex, bool partialHue)
{
    /*
    There are three types of art images; land, static, and UO alpha
    Land images are of flat terrain.
    Static images are of items (walls, equipment, food, etc)
    UO alpha images are a sort of animated characters, they used these instead of Anim.mul in the UO alpha.

    Static and UO alpha images share the same format, while Land images use another.
    */

    std::vector<char> dataVec;
    if (!getClassicPixelData(drawFromUOP, id, &dataVec))
        return nullptr;

    int16_t width = 0, height = 0;
    QImage* img = nullptr;

    size_t READMEM_dataOffset = 0;
    const char* READMEM_dataPtr = dataVec.data();
#define READMEM(dest, size) \
    memcpy((void*)&(dest), (const void*)(READMEM_dataPtr+READMEM_dataOffset), (size)); \
    READMEM_dataOffset += (size);

    if (id < kItemsOffset)
    {
        // Read land tile

        /*
        It's a "raw" tile, there's no offset-run encoding. The idx lookup points only to:
        WORD[1024] imageColors;

        Land images are fixed size at 44x44.
        They also have fixed transparent locations, and the format is optimized for this.
        The data stored for a tile resembles a square block, rotated 45 degrees.
        Therefore, the tile is loaded from the tip, down to the widest portion, then down to the bottom tip as opposed to a straight matrix.
        WORD imageColors, in the series:
        2, 4, 6, 8, 10 ... 40, 42, 44, 44, 42, 40 ... 10, 8, 6, 4, 2

        To read the first 22 lines, you first initialize X to 22, Y to 0, and LineWidth to 2
        Then repeat the following 22 times:
        Decrease X by 1
        Read and Draw (LineWidth) number of pixels
        Increase Y by 1
        Increase LineWidth by 2

        For the last 22 lines, do the following:
        Read and Draw (LineWidth) number of pixels
        Increase X by 1
        Increase Y by 1
        Decrease LineWidth by 2
        The resulting image is the diamond shaped tile.
        */

        width = height = 44;
        img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
        img->fill(0);

        // Algorithm from Punt's C++ Ultima SDK
        uint16_t rawcolor_argb16 = 0;
        int X = 22;
        int Y = 0;
        int linewidth = 2;
        for (int temp = 0; temp < 22; ++temp)
        {
            --X;
            for (int draw = 0; draw < linewidth; ++draw)
            {
                READMEM(rawcolor_argb16, 2);
                ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                //if ( (color &0x7FF) == 0 || (color & 0x7FF)==0x7FF )
                //  continue;
                if (hueIndex > 0)   // client starts to count from 1 (0 means do not change the color)
                {
                    UOHueEntry hue = m_UOHues->getHueEntry(hueIndex-1);
                    color_argb16 = hue.applyToColor(color_argb16, partialHue);
                }
                ARGB32 color_argb32 = convert_ARGB16_to_ARGB32(color_argb16);
                img->setPixel(X+draw, Y, color_argb32.getVal());
            }
            ++Y ;
            linewidth += 2;
        }

        X = 0;
        linewidth = 44;
        Y = 22;
        for (int temp = 0; temp < 22; ++temp)
        {
            for (int draw = 0; draw < linewidth; ++draw)
            {
                READMEM(rawcolor_argb16, 2);
                ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                //if ( (color &0x7FF) == 0 || (color & 0x7FF)==0x7FF )
                //  continue;
                if (hueIndex > 0)   // client starts to count from 1 (0 means do not change the color)
                {
                    UOHueEntry hue = m_UOHues->getHueEntry(hueIndex-1);
                    color_argb16 = hue.applyToColor(color_argb16, partialHue);
                }
                ARGB32 color_argb32 = convert_ARGB16_to_ARGB32(color_argb16);
                img->setPixel(X+draw, Y, color_argb32.getVal());
            }
            ++X;
            ++Y;
            linewidth -= 2;
        }
    }
    else
    {
        // Read static tile

        /*
        DWORD header; // Unknown
        WORD width;
        WORD height;
        WORD[height] lookupTable;
        Offset-Run data...

        The lookup table is offset from the data start, and treats the data by 16-bits.
        To get the proper byte offset, you need to add 4 + height, then multiply by two.
        The static images are compressed with an offset-run encoding.

        Offset-Run data:
        WORD xOffset;
        WORD xRun;
        WORD[xRun] runColors;

        If xOffset is 0 and xRun is 0, the line has been completed. If not, continue reading the chunks.
        Processing the Offset-Run data is simple:
        Increase X by xOffset
        Foreach xRun
        Read and Draw the next pixel
        Increase X by 1
        End Foreach

        When the line is completed, simply reset X to 0, increase Y, and seek to the next lookup in the lookupTable array and continue.
        */

        uint32_t flags = 0;
        READMEM(flags, 4);
        READMEM(width, 2);
        READMEM(height, 2);

        // Get the lookup table
        uint16_t* lookups = new uint16_t[height];
        READMEM(*lookups, 2*height);
        size_t datastart = READMEM_dataOffset;

        // Draw art
        img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
        img->fill(0);

        // Algorithm from Punt's C++ Ultima SDK
        for (int Y=0, X=0; Y < height ; ++Y)
        {
            X=0;

            uint16_t lk = lookups[Y];
            READMEM_dataOffset = lk * 2 + datastart;
            uint16_t xOffset = 1, xRun = 1;
            while (xOffset + xRun != 0)
            {
                READMEM(xOffset, 2);
                READMEM(xRun, 2);
                if (xOffset + xRun != 0)
                {
                    X += xOffset ;
                    for (unsigned jj=0; jj < xRun; ++jj)
                    {
                        uint16_t rawcolor_argb16 = 0;
                        READMEM(rawcolor_argb16, 2);
                        ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                        if (hueIndex > 0)   // client starts to count from 1 (0 means do not change the color)
                        {
                            UOHueEntry hue = m_UOHues->getHueEntry(hueIndex-1);
                            color_argb16 = hue.applyToColor(color_argb16, partialHue);
                        }
                        ARGB32 color_argb32 = convert_ARGB16_to_ARGB32(color_argb16);
                        img->setPixel(X, Y, color_argb32.getVal());
                        ++X;
                    }
                }
            }
        }

        delete[] lookups;
    }

    return img;
}

#undef READMEM
}
