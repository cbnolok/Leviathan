#ifndef KEYSTROKESENDER_COMMON_H
#define KEYSTROKESENDER_COMMON_H

#include <string>
#include <vector>

namespace ks
{


enum class UOClientType
{
    Unknown  = -1,
    Classic  = 0,
    Enhanced = 1,
};

enum class KSError
{
    Ok         = 0,    // no error
    NoWindow,
    StringShort,
};

extern const char * const UOClientWindowTitles[];
extern const char * const KSErrorString[];


}

#endif // KEYSTROKESENDER_COMMON_H
