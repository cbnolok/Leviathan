#ifndef KEYSTROKESENDER_COMMON_H
#define KEYSTROKESENDER_COMMON_H

#include <string>
#include <vector>

namespace ks
{


enum class UOClientType
{
    Unknown     = -1,
    Classic     = 0,
    Enhanced    = 1,
    ThirdParty  = 2
};

enum class KSError
{
    Ok          = 0,    // no error
    NoWindow,
    StringShort,
};


extern const char * const KSErrorString[];

UOClientType detectClientType(std::string windowName, std::string windowNameThirdpartyFragment);

}

#endif // KEYSTROKESENDER_COMMON_H
