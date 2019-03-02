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

    ARGB16 getLandColor16(unsigned int index);
    ARGB32 getLandColor32(unsigned int index);

    ARGB16 getItemColor16(unsigned int index);
    ARGB32 getItemColor32(unsigned int index);

private:
    static const int kEntriesCount = 0x8000;
    ARGB16 m_colors16[kEntriesCount];
    ARGB32 m_colors32[kEntriesCount];
};

}

#endif // UORADARCOL_H
