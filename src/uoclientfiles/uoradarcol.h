#ifndef UORADARCOL_H
#define UORADARCOL_H

#include "uohues.h"
#include <string>

namespace uocf
{

class UORadarCol
{
public:
    UORadarCol(const std::string& filePath);

    ARGB16 getLandColor16(unsigned int index) const;
    ARGB32 getLandColor32(unsigned int index) const;

    ARGB16 getItemColor16(unsigned int index) const;
    ARGB32 getItemColor32(unsigned int index) const;

private:
    static constexpr unsigned int kEntriesCount = 0x8000;
    ARGB16 m_colors16[kEntriesCount];
    ARGB32 m_colors32[kEntriesCount];
};

}

#endif // UORADARCOL_H
