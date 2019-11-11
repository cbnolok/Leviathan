#include "uostatics.h"
#include "exceptions.h"
#include <cstring> // for memcpy

namespace uocf
{


std::vector<StaticsEntry> StaticsBlock::getItemsAtOffsets(unsigned char xOffset, unsigned char yOffset) const
{
    std::vector<StaticsEntry> retVec;
    for (unsigned i = 0; i < entriesCount; ++i)
    {
        const StaticsEntry& cur = entries[i];
        if ((cur.xOffset == xOffset) && (cur.yOffset == yOffset))
            retVec.emplace_back(cur);
    }
    return retVec;
}

std::vector<StaticsEntry> StaticsBlock::getItemsAtOffsets(unsigned char xOffset, unsigned char yOffset, char z) const
{
    std::vector<StaticsEntry> retVec;
    for (unsigned i = 0; i < entriesCount; ++i)
    {
        const StaticsEntry& cur = entries[i];
        if ((cur.xOffset == xOffset) && (cur.yOffset == yOffset) && (cur.z == z))
            retVec.emplace_back(cur);
    }
    return retVec;
}

bool StaticsBlock::getTopItem(StaticsEntry *entry, unsigned char xOffset, unsigned char yOffset) const
{
    /*
    std::vector<StaticsEntry> entriesAtOff = getItemsAtOffsets(xOffset, yOffset);
    if (entriesAtOff.empty())
        return false;

    *entry = entriesAtOff[0];
    for (unsigned i = 0; i < entriesCount; ++i)
    {
        const StaticsEntry& cur = entries[i];
        if (cur.z > entry->z)
            *entry = cur;
    }
    */

    const StaticsEntry *highest = nullptr;
    for (unsigned i = 0; i < entriesCount; ++i)
    {
        const StaticsEntry& cur = entries[i];
        if ((cur.xOffset == xOffset) && (cur.yOffset == yOffset) && (!highest || (cur.z > highest->z)))
            highest = &cur;
    }

    if (!highest)
        return false;

    *entry = *highest;
    return true;
}


UOStatics::UOStatics(const std::string &clientPath, unsigned int fileIndex, unsigned int mapWidth, unsigned int mapHeight) :
    m_clientPath(clientPath), m_fileIndex(fileIndex), m_mapWidth(mapWidth), m_mapHeight(mapHeight),
    m_staidx(clientPath + "/staidx" + std::to_string(m_fileIndex) + ".mul")
{
    openStream();
    closeStream();
}

void UOStatics::openStream()
{
    std::string filePath(m_clientPath + "/statics" + std::to_string(m_fileIndex) + ".mul");
    m_stream.open(filePath, std::ifstream::in | std::ifstream::binary);
    if (!m_stream.is_open())
        throw InvalidStreamException("UOStatics", "Couldn't open file.");
}

void UOStatics::closeStream()
{
    if (!m_stream.is_open())
        throw InvalidStreamException("UOStatics", "Trying to close an already closed stream.");
    m_stream.close();
}

void UOStatics::clearIdxCache()
{
    m_staidx.clearCache();
}

void UOStatics::cacheIdxData()
{
    m_staidx.cacheData();
}

unsigned int UOStatics::getBlockIndex(unsigned int xTile, unsigned int yTile) const noexcept
{
    const unsigned xBlock = xTile / StaticsBlock::kTilesPerRow;
    const unsigned yBlock = yTile / StaticsBlock::kTilesPerColumn;
    const unsigned yBlockCount = m_mapHeight / StaticsBlock::kTilesPerColumn;
    return (xBlock * yBlockCount) + yBlock;
}

UOIdx::Entry UOStatics::readIdxToBlock(unsigned int xTile, unsigned int yTile)
{
    const unsigned nBlock = getBlockIndex(xTile, yTile);
    UOIdx::Entry idxEntry;
    if (m_staidx.getLookup(nBlock, &idxEntry))
        return idxEntry;
    return {};
}

UOIdx::Entry UOStatics::readIdxToBlock(unsigned int index)
{
    UOIdx::Entry idxEntry;
    if (m_staidx.getLookup(index, &idxEntry))
        return idxEntry;
    return {};
}

StaticsBlock UOStatics::readBlock(const UOIdx::Entry &idxEntry)
{
    if (!m_stream.is_open())
        throw InvalidStreamException("UOStatics", "readBlock accessing closed stream.");

    StaticsBlock block;
    const unsigned entriesCount = (idxEntry.size / StaticsEntry::kSize);
    block.entriesCount = entriesCount;
    if (entriesCount == 0)
    {
        block.initialized = true;
        return block;
    }
    block.initialized = false;
    block.entries = std::make_unique<StaticsEntry[]>(entriesCount);

    m_stream.seekg(idxEntry.lookup);
    /*
    for (unsigned i = 0; i < nEntries; ++i)
    {
        m_stream.read(reinterpret_cast<char*>(&block.entries[i].id), 2);
        m_stream.read(reinterpret_cast<char*>(&block.entries[i].xOffset), 1);
        m_stream.read(reinterpret_cast<char*>(&block.entries[i].yOffset), 1);
        m_stream.read(reinterpret_cast<char*>(&block.entries[i].z), 1);
        m_stream.read(reinterpret_cast<char*>(&block.entries[i].hue), 2);
    }
    */
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(idxEntry.size);
    char* bufPtr = buf.get();
    m_stream.read(bufPtr, std::streamsize(idxEntry.size));
    for (unsigned i = 0; i < entriesCount; ++i)
    {
        StaticsEntry& entry = block.entries[i];
        memcpy(&entry.id,        bufPtr, 2);  bufPtr += 2;
        memcpy(&entry.xOffset,   bufPtr, 1);  bufPtr += 1;
        memcpy(&entry.yOffset,   bufPtr, 1);  bufPtr += 1;
        memcpy(&entry.z,         bufPtr, 1);  bufPtr += 1;
        memcpy(&entry.hue,       bufPtr, 2);  bufPtr += 2;
    }

    if (!m_stream.good())
        throw InvalidStreamException("UOStatics", "readBlock I/O error.");

    block.initialized = true;
    return block;
}


// since we want the modulo for a power of 2 (8, in this case), we can use the AND operator for 8 - 1
#define BLOCK_OFFSET_X(x)   static_cast<unsigned char>(x & (StaticsBlock::kTilesPerRow - 1))
#define BLOCK_OFFSET_Y(y)   static_cast<unsigned char>(y & (StaticsBlock::kTilesPerColumn - 1))

std::vector<StaticsEntry> UOStatics::getItemsAtCoordsFromBlock(const StaticsBlock& block, unsigned int x, unsigned int y) const
{
    return block.getItemsAtOffsets(BLOCK_OFFSET_X(x), BLOCK_OFFSET_Y(y));
}

std::vector<StaticsEntry> UOStatics::getItemsAtCoordsFromBlock(const StaticsBlock& block, unsigned int x, unsigned int y, char z) const
{
    return block.getItemsAtOffsets(BLOCK_OFFSET_X(x), BLOCK_OFFSET_Y(y), z);
}

bool UOStatics::getTopItemFromBlock(StaticsEntry *entry, const StaticsBlock& block, unsigned int x, unsigned int y) const
{
    return block.getTopItem(entry, BLOCK_OFFSET_X(x), BLOCK_OFFSET_Y(y));
}


}
