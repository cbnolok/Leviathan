#ifndef UOMAP_H
#define UOMAP_H

#include "uostatics.h"
#include <functional>

class QRect;
class QImage;


namespace uocf
{

class UORadarCol;
class UOHues;


struct MapCell
{
    static constexpr unsigned int kSize = 2 + 1;

    unsigned short id;
    char z;
};

struct MapBlock
{
    static constexpr unsigned int kCellsPerRow = 8;
    static constexpr unsigned int kCellsPerColumn = 8;
    static constexpr unsigned int kCellsPerBlock = kCellsPerRow * kCellsPerColumn;
    static constexpr unsigned int kSize = 4 + (kCellsPerBlock * MapCell::kSize);
    bool initialized;

    unsigned int header;
    // Each block contains 64 cells, treated as an 8x8 matrix loaded left to right, top to bottom.
    MapCell cells[kCellsPerBlock];
};


class UOMap
{
public:
    static constexpr unsigned int kMaxSupportedMap = 5;
    static constexpr unsigned int kScaleFactorMin = 0;
    static constexpr unsigned int kScaleFactorMax = 4;

    // better using a virtually unused color on the map, to avoid unnecessary redrawings
    static constexpr unsigned int kUninitializedRGB = 0x00ff70; //0xF0F0F0; // aa(ignore alpha value) rr gg bb


    UOMap(const std::string& clientPath, unsigned int fileIndex);
    UOMap(const std::string& clientPath, unsigned int fileIndex, unsigned int width, unsigned int height); // for custom sized maps
    void setCachePointers(UORadarCol* radarcol, UOStatics* statics_optional = nullptr, UOHues* hues_optional = nullptr);

    inline bool isStreamOpened() const {
        return m_stream.is_open();
    }
    void openStream();
    void closeStream();
    void setupDataCache();
    void freeDataCache();

    inline unsigned int getWidth() const {
        return m_width;
    }
    inline unsigned int getHeight() const {
        return m_height;
    }

    unsigned int getBlockIndex(unsigned int xTile, unsigned int yTile) const noexcept;
    const MapBlock*     getCacheMapBlock(unsigned int x, unsigned int y);
    const StaticsBlock *getCacheStaticsBlock(unsigned int x, unsigned int y);

    const MapCell&  getCellFromBlock(const MapBlock& block, unsigned int xTile, unsigned int yTile) const;
    const MapCell&  readCell(unsigned int xTile, unsigned int yTile);
    MapBlock readBlock(unsigned int index);

    char getTopZAtXY(unsigned int x, unsigned int y);

    static void scaleCoordsMapToImage(unsigned int scaleFactor, unsigned int *x, unsigned int *y);
    static void scaleCoordsImageToMap(unsigned int scaleFactor, unsigned int *x, unsigned int *y);
    void clipCoordsToMapSize(unsigned int *xMapStart, unsigned int *yMapStart, unsigned int *width, unsigned int *height);
    bool drawRectInImage(QImage *image, int xImageOffset, int yImageOffset,
                         std::function<void (int)> reportProgress,
                         unsigned int xMapStart, unsigned int yMapStart, unsigned int width, unsigned int height,
                         unsigned int scaleFactor = 1, bool drawStatics = true);
    QImage* drawRect(std::function<void (int)> reportProgress,
                     unsigned int xMapStart, unsigned int yMapStart, unsigned int width, unsigned int height,
                     unsigned int scaleFactor = 1, bool drawStatics = true);

private:
    std::string m_clientPath;
    std::string m_filePath;
    unsigned int m_fileIndex;
    unsigned int m_width, m_height;

    std::ifstream m_stream;
    UORadarCol *m_UORadarcol;
    UOStatics *m_UOStatics;
    UOHues *m_UOHues;
    //UOTiledata

    unsigned int m_cachedMapBlocksCount;
    std::unique_ptr<MapBlock[]> m_cachedMapBlocks;
    unsigned int m_cachedStaticsBlocksCount;
    std::unique_ptr<StaticsBlock[]> m_cachedStaticsBlocks;
};


}

#endif // UOMAP_H
