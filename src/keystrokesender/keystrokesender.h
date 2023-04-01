#ifndef KEYSTROKESENDER_H
#define KEYSTROKESENDER_H

#if defined(_WIN32)
    #include "keystrokesender_windows.h"
#elif defined(__APPLE__)
    #include "keystrokesender_mac.h"
#elif defined(__unix__)
    #include "keystrokesender_linux.h"
#endif

#endif // KEYSTROKESENDER_H
