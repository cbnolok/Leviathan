#ifndef KEYSTROKESENDER_H
#define KEYSTROKESENDER_H

#include "keystrokesender_windows.h"
#include "keystrokesender_linux.h"
#include <string>


namespace keystrokesender
{


#ifdef _WIN32
class KeystrokeSender : public KeystrokeSender_Windows
#else
class KeystrokeSender : public KeystrokeSender_Linux
#endif
{
public:
    #ifdef _WIN32
    KeystrokeSender(bool setFocusToWindow = false) : KeystrokeSender_Windows(setFocusToWindow) {};
    #else
    KeystrokeSender(bool setFocusToWindow = false) : KeystrokeSender_Linux(setFocusToWindow) {};
    #endif

    void setSetFocusToWindow(bool value);

    KSError getError() const;
    std::string getErrorString() const;
    UOClientType getClientType() const;

    // Public methods inherited from KeystrokeSender_Windows and KeystrokeSender_Linux:
    // sendChar();
    // sendEnter();
    // sendString();
};


}

#endif // KEYSTROKESENDER_H
