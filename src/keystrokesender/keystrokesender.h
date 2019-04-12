#ifndef KEYSTROKESENDER_H
#define KEYSTROKESENDER_H

#include "keystrokesender_windows.h"
#include "keystrokesender_mac.h"
#include "keystrokesender_linux.h"


namespace ks
{

// Namespace functions
const char * getErrorStringStatic(KSError err);

// Class
#if BUILD_WINDOWS
    #define KS_BASE_CLASS KeystrokeSender_Windows
#elif BUILD_MACOS
    #define KS_BASE_CLASS KeystrokeSender_Mac
#elif BUILD_LINUX
    #define KS_BASE_CLASS KeystrokeSender_Linux
#endif

class KeystrokeSender : public KS_BASE_CLASS
{
public:
    KeystrokeSender(bool setFocusToWindow = false) : KS_BASE_CLASS(setFocusToWindow) {}

    void setSetFocusToWindow(bool value);

    KSError getError() const;
    std::string getErrorString() const;
    UOClientType getClientType() const;

    // Public methods inherited from KS_BASE_CLASS:
    // sendChar();
    // sendEnter();
    // sendString();
    // sendStrings();

    // Public static methods inherited from KS_BASE_CLASS:
    // sendCharFast();
    // sendEnterFast();
    // sendStringFast();
    // sendStringsFast();
    //-- Async methods
    // sendCharFastAsync();
    // sendEnterFastAsync();
    // sendStringFastAsync();
    // sendStringsFastAsync();
};

#undef KS_BASE_CLASS


}

#endif // KEYSTROKESENDER_H
