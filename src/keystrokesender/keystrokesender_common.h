#ifndef KEYSTROKESENDER_COMMON_H
#define KEYSTROKESENDER_COMMON_H

#include <string>
#include <vector>

namespace ks
{


enum UOClientType
{
    CLIENT_UNK      = -1,
    CLIENT_CLASSIC  = 0,
    CLIENT_ENHANCED = 1,
};

enum KSError
{
    KSERR_OK         = 0,    // no error
    KSERR_NOWINDOW,
    KSERR_STRINGSHORT,
};

extern const char * const UOClientWindowTitles[];
extern const char * const KSErrorString[];


}

#endif // KEYSTROKESENDER_COMMON_H
