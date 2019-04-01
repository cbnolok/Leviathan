#include "uohues.h"
#include <fstream>
#include "exceptions.h"

#include "../globals.h"
#define LOG(x) appendToLog(x)


namespace uocf
{

UOHues::UOHues(const std::string& huesPath) : m_hues{}
{
    LOG("Loading hues.mul");

    std::ifstream fin;
    fin.open(huesPath, std::ifstream::in | std::ifstream::binary);
    if (!fin.is_open())
    {
        LOG("Error opening hues.mul"); // EXC
        return;
    }

    /*
    HueGroup
    --------
    DWORD Header;
    HueEntry Entries[8];

    HueEntry
    --------
    WORD ColorTable[32];
    WORD TableStart;
    WORD TableEnd;
    CHAR Name[20];
    */
    // Just read in HueGroups until you hit the end of the file.
    for (unsigned int hue_num = 0; hue_num < kHuesCount;)
    {
        fin.seekg(4, std::ifstream::cur);   // Skip Header of the hue group
        for (unsigned int j = 0; j < 8; ++j)    // Iterate through the 8 HueEntries of this HueGroup
        {
            //hues[hue_num].index = hue_num;

            // Fill the ColorTable
            uint16_t colorTableRaw[UOHueEntry::kTableColorsCount] = {};
            fin.read(reinterpret_cast<char*>(colorTableRaw), 2 * UOHueEntry::kTableColorsCount);
            for (unsigned int k = 0; k < UOHueEntry::kTableColorsCount; ++k)
            {
                m_hues[hue_num].colorTable16[k] = ARGB16(colorTableRaw[k]);
                m_hues[hue_num].colorTable32[k] = convert_ARGB16_to_ARGB32(m_hues[hue_num].colorTable16[k]);
            }

            // Skip TableStart and TableEnd
            fin.seekg(4, std::ifstream::cur);

            // Read the Hue Name
            //memset(hues[hue_num].name, 0, sizeof(hues[hue_num].name));
            fin.read(reinterpret_cast<char*>(&m_hues[hue_num].name), sizeof(m_hues[hue_num].name));

            if (fin.eof())
                throw MalformedFileException("UOHues", "hues.mul");

            ++hue_num;
        }
    }
}

/*
UOHues::~UOHues()
{

}
*/

const UOHueEntry& UOHues::getHueEntry(unsigned int index) const
{
    index &= 0x3FFF;

    // for the client: 0 is no hue
    // for hues.mul: 0 is the first entry
    // so the client starts to count from 1
    if (index < kHuesCount)
        return m_hues[index];

    return m_hues[0];
}

std::string UOHueEntry::getName() const
{
    return std::string(name);
}

ARGB16 UOHueEntry::getColor16(unsigned int index) const
{
    if (index >= kTableColorsCount)
        return ARGB16(0);
    return colorTable16[index];
}

ARGB32 UOHueEntry::getColor32(unsigned int index) const
{
    if (index >= kTableColorsCount)
        return ARGB32(0);
    return colorTable32[index];
}

ARGB16 UOHueEntry::applyToColor16(ARGB16 color16, bool applyToGrayOnly) const
{
    /*
      Hues are applied to an image in one of two ways. Either all gray pixels are mapped
      to the specified color range (resulting in a part of an image changed to that color)
      or all pixels are first mapped to grayscale and then the specified color range is mapped over the entire image.
    */

    // If it's full transparent, do i have to change the color?
    //if (color16.getA() == 0)
    //    return color16;

    if (applyToGrayOnly)
    {
        if (color16.getA() == 0)
            return color16;
        else if ( !((color16.getR() == color16.getG()) && (color16.getR() == color16.getB())) )
            return color16;
    }

    /*  -- The AxisII Way -- */         // Dunno why, this way the image looks bad
    // Grayscale the input color
    //unsigned int ret = color16.getR() + color16.getG() + color16.getB();
    //ret /= 3;   // will always be < 32

    //return colorTable16[ret];

    /* -- UOFiddler/Punt's Way -- */
    return colorTable16[color16.getR()];   // ret will always be < 32
}

ARGB32 UOHueEntry::applyToColor32(ARGB32 color32, bool applyToGrayOnly) const
{
    if (applyToGrayOnly)
    {
        if (color32.getA() == 0)
            return color32;
        else if ( !((color32.getR() == color32.getG()) && (color32.getR() == color32.getB())) )
            return color32;
    }

    return colorTable32[convert_ARGB32_to_ARGB16(color32).getR()];
}

}
