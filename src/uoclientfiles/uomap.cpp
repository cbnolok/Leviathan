#include "uomap.h"

#include <QImage>
#include <cmath>

#include "exceptions.h"
#include "uoradarcol.h"
#include "uostatics.h"


namespace uocf
{


unsigned int UOMap::getWidth() const {
    return m_width;
}
unsigned int UOMap::getHeight() const {
    return m_height;
}


UOMap::UOMap(const std::string& clientPath, unsigned int fileIndex) :
    m_clientPath(clientPath),
    m_fileIndex(fileIndex),
    m_UORadarcol(nullptr), m_UOStatics(nullptr), m_UOHues(nullptr)
{
    m_filePath = m_clientPath + "/map" + std::to_string(fileIndex) + ".mul";

    openStream();
    m_stream.seekg(0, std::ifstream::end);
    std::streamoff size = m_stream.tellg();
    closeStream();

    switch (fileIndex)
    {
        case 0:
        case 1:
            if (size <= 77070336) // pre-ML
            {
                m_width = 6144;
                m_height = 4096;
            }
            else
            {
                m_width = 7168;
                m_height = 4096;
            }
            break;

        case 2:
            m_width = 2304;
            m_height = 1600;
            break;

        case 3:
            m_width = 2560;
            m_height = 2048;
            break;

        case 4:
            m_width = 1448;
            m_height = 1448;
            break;

        case 5:
            m_width = 1280;
            m_height = 4096;
            break;

        default:
            throw UnsupportedActionException("UOMap", m_filePath);
    }

    int blocks = (m_width * m_height) / MapBlock::kCellsPerBlock;
    std::streamoff expectedSize = MapBlock::kSize * blocks;

    if (size < expectedSize)
        throw MalformedFileException("UOMap", m_filePath);
}


UOMap::UOMap(const std::string& clientPath, unsigned int fileIndex, unsigned int width, unsigned int height) :
    m_clientPath(clientPath),
    m_fileIndex(fileIndex),
    m_width(width), m_height(height),
    m_UORadarcol(nullptr), m_UOStatics(nullptr), m_UOHues(nullptr)
{
    m_filePath = m_clientPath + "/map" + std::to_string(fileIndex) + ".mul";

    openStream();
    m_stream.seekg(0, std::ifstream::end);
    std::streamoff size = m_stream.tellg();
    closeStream();

    int blocks = (m_width * m_height) / MapBlock::kCellsPerBlock;
    std::streamoff expectedSize = MapBlock::kSize * blocks;

    if (size < expectedSize)
        throw MalformedFileException("UOMap", m_filePath);
}

void UOMap::setCachePointers(UORadarCol* radarcol, UOStatics *statics_optional, UOHues *hues_optional)
{
    m_UORadarcol = radarcol;
    m_UOStatics = statics_optional;
    m_UOHues = hues_optional;
}


void UOMap::openStream()
{
    m_stream.open(m_filePath, std::ifstream::in | std::ifstream::binary);
    if (!m_stream.is_open())
        throw InvalidStreamException("UOMap", "Couldn't open file.");
}

void UOMap::closeStream()
{
    if (!m_stream.is_open())
        throw InvalidStreamException("UOMap", "Trying to close an already closed stream.");
    m_stream.close();
}

void UOMap::freeDataCache()
{
    m_cachedMapBlocks.clear();
    m_cachedStaticsBlocks.clear();
}


unsigned int UOMap::getBlockIndex(unsigned int xTile, unsigned int yTile) const noexcept
{
    unsigned xBlock = xTile / MapBlock::kCellsPerRow;
    unsigned yBlock = yTile / MapBlock::kCellsPerColumn;
    unsigned yBlockCount = m_height / MapBlock::kCellsPerColumn;
    return (xBlock * yBlockCount) + yBlock;
}

const MapBlock* UOMap::getCacheMapBlock(unsigned int x, unsigned int y)
{
    unsigned int index = getBlockIndex(x, y);

    const MapBlock *mapBlock;
    auto itMapFind = m_cachedMapBlocks.find(index);
    if (itMapFind == m_cachedMapBlocks.end())
    {
        bool openClose = !isStreamOpened();
        if (openClose)
            openStream();

        auto itInsert = m_cachedMapBlocks.insert(std::make_pair(index, readBlock(index)));
        mapBlock = &itInsert.first->second;

        if (openClose)
            closeStream();
    }
    else
    {
        mapBlock = &itMapFind->second;
    }
    return mapBlock;
}

const StaticsBlock* UOMap::getCacheStaticsBlock(unsigned int x, unsigned int y)
{
    const UOIdx::Entry staticsBlockIdxEntry = m_UOStatics->readIdxToBlock(x, y);
    if (staticsBlockIdxEntry.lookup == UOIdx::Entry::kInvalid)
        return nullptr;

    const StaticsBlock *staticsBlock;
    auto itStaticsFind = m_cachedStaticsBlocks.find(staticsBlockIdxEntry.lookup);
    if (itStaticsFind == m_cachedStaticsBlocks.end())
    {
        bool openClose = !m_UOStatics->isStreamOpened();
        if (openClose)
            m_UOStatics->openStream();

        auto itInsert = m_cachedStaticsBlocks.insert(std::make_pair(staticsBlockIdxEntry.lookup, m_UOStatics->readBlock(staticsBlockIdxEntry)));
        staticsBlock = &itInsert.first->second;

        if (openClose)
            m_UOStatics->closeStream();
    }
    else
    {
        staticsBlock = &itStaticsFind->second;
    }
    return staticsBlock;
}


const MapCell& UOMap::getCellFromBlock(const MapBlock& block, unsigned int xTile, unsigned int yTile) const
{
    unsigned xCell = xTile % MapBlock::kCellsPerRow;
    unsigned yCell = yTile % MapBlock::kCellsPerColumn;
    return block.cells[(yCell * MapBlock::kCellsPerColumn) + xCell];
}

const MapCell& UOMap::readCell(unsigned int xTile, unsigned int yTile)
{
    if (!m_stream.is_open())
        throw InvalidStreamException("UOMap", "getCell accessing closed stream.");

    const MapBlock& block = readBlock(getBlockIndex(xTile, yTile));
    return getCellFromBlock(block, xTile, yTile);
}

MapBlock UOMap::readBlock(unsigned int index)
{
    if (!m_stream.is_open())
        throw InvalidStreamException("UOMap", "readBlock accessing closed stream.");

    static const int kMapBlockSize = 4 + (MapBlock::kCellsPerBlock * 3);
    m_stream.seekg(index * kMapBlockSize);

    MapBlock block = {};
    m_stream.read(reinterpret_cast<char*>(&block.header), 4);
    for (unsigned i = 0; i < MapBlock::kCellsPerBlock; ++i)
    {
        m_stream.read(reinterpret_cast<char*>(&block.cells[i].id), 2);
        m_stream.read(reinterpret_cast<char*>(&block.cells[i].z), 1);
    }
    return block;
}


char UOMap::getTopZAtXY(unsigned int x, unsigned int y)
{
    const MapBlock *mb = getCacheMapBlock(x, y);
    const StaticsBlock *sb = !m_UOStatics ? nullptr : getCacheStaticsBlock(x, y);

    const char zTile = getCellFromBlock(*mb, x, y).z;
    if (sb)
    {
        StaticsEntry se;
        if (m_UOStatics->getTopItemFromBlock(&se, *sb, x, y))
            return (zTile > se.z) ? zTile : se.z;
    }
    return zTile;
}


void UOMap::scaleCoordsMapToImage(unsigned int scaleFactor, unsigned int *x, unsigned int *y) // static
{
    //if (!x || !y)
    //    return;
    const double f = pow(2, scaleFactor);
    *x = unsigned(*x / f);
    *y = unsigned(*y / f);
}

void UOMap::scaleCoordsImageToMap(unsigned int scaleFactor, unsigned int *x, unsigned int *y) // static
{
    //if (!x || !y)
    //    return;
    const double f = pow(2, scaleFactor);
    *x = unsigned(*x * f);
    *y = unsigned(*y * f);
}

void UOMap::clipCoordsToMapSize(unsigned int *xMapStart, unsigned int *yMapStart, unsigned int *width, unsigned int *height)
{
    if (*xMapStart > m_width)
        *xMapStart = 0;
    if ((*xMapStart + *width) > m_width)
        *width = m_width - *xMapStart;
    if (*yMapStart > m_height)
        *yMapStart = 0;
    if ((*yMapStart + *height) > m_height)
        *height = m_height - *yMapStart;
}

bool UOMap::drawRectInImage(QImage *image, int xImageOffset, int yImageOffset, const std::function<void (int)> &reportProgress,
                            unsigned int xMapStart, unsigned int yMapStart, unsigned int width, unsigned int height,
                            unsigned int scaleFactor, bool drawStatics)
{
    if (!m_UORadarcol)
        throw NoCachePtrException("UOMap");

    if (scaleFactor > kScaleFactorMax)
        scaleFactor = kScaleFactorMax;
    //else if (scaleFactor < kScaleFactorMin)
    //    scaleFactor = kScaleFactorMin;

    if (drawStatics)
    {
        if (!m_UOStatics)
            drawStatics = false;
        else if (!m_UOStatics->hasIdxCache())
            m_UOStatics->cacheIdxData();
    }

    const int destImageWidth = image->width(), destImageHeight = image->height();

    unsigned widthScaled = width, heightScaled = height;
    scaleCoordsMapToImage(scaleFactor, &widthScaled, &heightScaled);

    const unsigned long long pixelsToDraw = widthScaled * heightScaled;
    unsigned long long pixelsDrawn = 0;
    int progressVal = 0;

    int xImage = 0, yImage = 0;
    int xTileRelative = -1, yTileRelative = -1;

    for (unsigned x = xMapStart, xEnd = xMapStart + width; x < xEnd; ++x)
    {
        if ((xImage >= int(widthScaled)) || (xImage >= destImageWidth))
            break;

        ++xTileRelative;
        if ((scaleFactor == 1) && (xTileRelative % 2))
            continue;
        if ((scaleFactor == 2) && (xTileRelative % 4))
            continue;
        if ((scaleFactor == 3) && (xTileRelative % 8))
            continue;
        if ((scaleFactor == 4) && (xTileRelative % 16))
            continue;

        yImage = -1;
        for (unsigned y = yMapStart, yEnd = yMapStart + height; y < yEnd; ++y)
        {
            if ((yImage >= int(heightScaled)) || (yImage >= destImageHeight))
                break;

            ++yTileRelative;
            if ((scaleFactor == 1) && (yTileRelative % 2))
                continue;
            if ((scaleFactor == 2) && (yTileRelative % 4))
                continue;
            if ((scaleFactor == 3) && (yTileRelative % 8))
                continue;
            if ((scaleFactor == 4) && (yTileRelative % 16))
                continue;

            ++yImage;
            const uint pixelVal = image->pixel(xImageOffset + xImage, yImageOffset + yImage);
            if ((pixelVal & 0x00FFFFFF) != kUninitializedRGB) // ignore alpha bits to check if the pixel color is pure white
                continue; // if not white (new qimage is manually initialized by fill(kUninitializedRGB)), no need to redraw existing pixel

            // Get map block
            if (!isStreamOpened())
                openStream();

            const MapBlock* mapBlock = getCacheMapBlock(x, y);

            // Get map cell
            const MapCell& mapCell = getCellFromBlock(*mapBlock, x, y);
            bool drawingLandtile = true;
            ARGB32 tileColor;

            if (drawStatics)
            {
                if (!m_UOStatics->isStreamOpened())
                    m_UOStatics->openStream();

                // Get statics block
                const StaticsBlock *staticsBlock = getCacheStaticsBlock(x, y);
                if (staticsBlock)
                {
                    // Get highest static tile at coordinates
                    StaticsEntry highestStatic;
                    if (m_UOStatics->getTopItemFromBlock(&highestStatic, *staticsBlock, x, y))
                    {
                        if (highestStatic.z >= mapCell.z)
                        {
                            drawingLandtile = false;
                            tileColor = m_UORadarcol->getItemColor32(highestStatic.id);

                            // Do we need to hue it?
                            if (m_UOHues && (highestStatic.hue > 0))
                                tileColor = m_UOHues->getHueEntry(highestStatic.hue - 1).applyToColor32(tileColor);
                        }
                    }
                }
            }
            if (drawingLandtile)
                tileColor = m_UORadarcol->getLandColor32(mapCell.id);

            // Draw tile
            image->setPixel(xImageOffset + xImage, yImageOffset + yImage, tileColor.getVal());

            // Report progress
            if (reportProgress)
            {
                ++pixelsDrawn;
                const int progressValNow = int( (pixelsDrawn * 100) / pixelsToDraw);
                if (progressValNow > progressVal)
                {
                    progressVal = progressValNow;
                    reportProgress(progressVal);
                }
            }
        }
        ++xImage;
    }

    if (isStreamOpened())
        closeStream();
    if (drawStatics && m_UOStatics->isStreamOpened())
            m_UOStatics->closeStream();

    return true;
}

QImage* UOMap::drawRect(const std::function<void (int)> &reportProgress,
                        unsigned int xMapStart, unsigned int yMapStart, unsigned int width, unsigned int height,
                        unsigned int scaleFactor, bool drawStatics)
{
    unsigned widthScaled = width, heightScaled = height;
    scaleCoordsMapToImage(scaleFactor, &widthScaled, &heightScaled);

    QImage *image = new QImage(int(widthScaled), int(heightScaled), QImage::Format_RGB32);
    image->fill(kUninitializedRGB);

    drawRectInImage(image, 0, 0,
                    reportProgress,
                    xMapStart, yMapStart, width, height,
                    scaleFactor, drawStatics);

    return image;
}

}
