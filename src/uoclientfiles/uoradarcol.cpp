#include "uoradarcol.h"
#include "exceptions.h"
#include "../globals.h"
#include <fstream>


namespace uocf
{

UORadarCol::UORadarCol(const std::string& filePath)
{
    appendToLog("Loading radarcol...");
    std::ifstream fin;
    fin.open(filePath, std::ifstream::in | std::ifstream::binary);

    uint16_t entries[kEntriesCount] = {};
    fin.read(reinterpret_cast<char*>(&entries), 2 * kEntriesCount);

    if (fin.eof())
        throw MalformedFileException("UORadarcol", "radarcol.mul");

    for (unsigned int i = 0; i < kEntriesCount; ++i)
    {
        m_colors16[i] = entries[i];
        m_colors32[i] = convert_ARGB16_to_ARGB32(m_colors16[i]);
    }
}


ARGB16 UORadarCol::getLandColor16(unsigned int index)
{
    if (index > 0x4000)
        index = 0;
    return m_colors16[index];
}

ARGB32 UORadarCol::getLandColor32(unsigned int index)
{
    if (index > 0x4000)
        index = 0;
    return m_colors32[index];
}


ARGB16 UORadarCol::getItemColor16(unsigned int index)
{
    if (index > 0x4000)
        index = 0x4000;
    return m_colors16[0x4000+index];
}

ARGB32 UORadarCol::getItemColor32(unsigned int index)
{
    if (index > 0x4000)
        index = 0x4000;
    return m_colors32[0x4000+index];
}


}
