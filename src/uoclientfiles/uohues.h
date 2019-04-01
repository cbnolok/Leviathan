#ifndef UOHUES_H
#define UOHUES_H

#include <string>
#include "colors.h"


namespace uocf
{

class UOHues;
struct UOHueEntry
{
    friend class UOHues;
    static constexpr unsigned int kTableColorsCount = 32;

private:
    //uint32_t index;
    char name[20];
    // convert the data once at loading time
    ARGB16 colorTable16[kTableColorsCount];
    ARGB32 colorTable32[kTableColorsCount];

public:
    ARGB16 getColor16(unsigned int index) const;
    ARGB32 getColor32(unsigned int index) const;
    std::string getName() const;

    ARGB16 applyToColor16(ARGB16 color16, bool applyToGrayOnly = false) const;  // apply hue to the RGB16 color
    ARGB32 applyToColor32(ARGB32 color32, bool applyToGrayOnly = false) const;  // apply hue to the RGB32 color
};


class UOHues
{
public:
    UOHues(const std::string &huesPath);  // loads hues
    //~UOHues();
    const UOHueEntry& getHueEntry(unsigned int index) const;

private:
    static constexpr unsigned int kHuesCount = 3000;
    UOHueEntry m_hues[kHuesCount];
};


}

#endif // UOHUES_H
