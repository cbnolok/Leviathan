#include "keystrokesender_common.h"

namespace ks
{

const char * const KSErrorString[]
{   // The string order must match the UOClientComError enum order.
    "No error.",
    "UO Client window was not found.",
    "String too short."
};

static const std::string UODefaultClientWindowTitles[]
{   // The string order must match the UOClientType enum order.
    "Ultima Online",      // for Classic Client
    "UOSA -",             // for Enhanced Client
    ""
};


UOClientType detectClientType(std::string windowName, std::string windowNameThirdpartyFragment) // static
{
    if (windowName.find(UODefaultClientWindowTitles[(int)UOClientType::Classic]) != std::string::npos)
    {
        return UOClientType::Classic;
    }
    if (windowName.find(UODefaultClientWindowTitles[(int)UOClientType::Enhanced]) != std::string::npos)
    {
        return UOClientType::Enhanced;
    }
    if (windowName.find(windowNameThirdpartyFragment) != std::string::npos)
    {
        return UOClientType::ThirdParty;
    }

    return UOClientType::Unknown;
}

}
