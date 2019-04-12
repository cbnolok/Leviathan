#include "keystrokesender_common.h"

namespace ks
{


const char * const UOClientWindowTitles[]
{   // The string order must match the UOClientType enum order.
    "Ultima Online",      // for Classic Client
    "UOSA -"              // for Enhanced Client
};

// TODO: do 3rd party clients (or using assistants with the official ones) have a different window title?

const char * const KSErrorString[]
{   // The string order must match the UOClientComError enum order.
    "No error.",
    "UO Client window was not found.",
    "String too short."
};


}
