#ifndef UOSTATICS_H
#define UOSTATICS_H

#include <vector>
#include <string>
#include <fstream>
#include "uoidx.h"

namespace uocf
{


struct StaticsEntry
{
    static const int kSize = 2 + 1 + 1 + 1 + 2;

    unsigned short id;
    unsigned char xOffset;
    unsigned char yOffset;
    char z;
    unsigned short hue;
};

struct StaticsBlock
{
    // Each block contains 64 tiles, treated as an 8x8 matrix loaded left to right, top to bottom.
    static const int kTilesPerRow = 8;
    static const int kTilesPerColumn = 8;
    static const int kTilesPerBlock = kTilesPerRow * kTilesPerColumn;

    std::vector<StaticsEntry> entries;

    std::vector<StaticsEntry> getItemsAtOffsets(unsigned char xOffset, unsigned char yOffset) const;
    std::vector<StaticsEntry> getItemsAtOffsets(unsigned char xOffset, unsigned char yOffset, char z)  const;
    bool getTopItem(StaticsEntry *entry, unsigned char xOffset, unsigned char yOffset) const;
};

struct UOStatics
{
    UOStatics(const std::string& clientPath, unsigned int fileIndex, unsigned int width, unsigned int height);

    void openStream();
    void closeStream();
    inline bool isStreamOpened() const {
        return m_stream.is_open();
    }

    bool hasIdxCache();
    void clearIdxCache();
    void cacheIdxData();

    unsigned int getBlockIndex(unsigned int xTile, unsigned int yTile) noexcept;
    UOIdx::Entry readIdxToBlock(unsigned int xTile, unsigned int yTile);
    StaticsBlock readBlock(const UOIdx::Entry& idxEntry);

    std::vector<StaticsEntry> getItemsAtCoordsFromBlock(const StaticsBlock& block, unsigned int x, unsigned int y) const;
    std::vector<StaticsEntry> getItemsAtCoordsFromBlock(const StaticsBlock& block, unsigned int x, unsigned int y, char z) const;
    bool getTopItemFromBlock(StaticsEntry *entry, const StaticsBlock& block, unsigned int x, unsigned int y) const;

private:
    std::string m_clientPath;
    unsigned int m_fileIndex;
    unsigned int m_mapWidth, m_mapHeight;
    std::ifstream m_stream;
    UOIdx m_staidx;
};


}

#endif // UOSTATICS_H
