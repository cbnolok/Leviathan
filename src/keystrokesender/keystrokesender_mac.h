#ifndef KEYSTROKESENDER_MAC_H
#define KEYSTROKESENDER_MAC_H

#if BUILD_MACOS

#include "keystrokesender_common.h"

namespace ks
{

class KeystrokeSender_Mac
{
protected:
    KeystrokeSender_Mac(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text

public:
    bool canSend();

    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const std::string& str, bool enterTerminated = true);
    bool sendStrings(const std::vector<std::string> &strings, bool enterTerminated = true);

    // Static methods: do the work without creating an instance of the class and setting everything manually
    static KSError sendCharFast(const char ch, bool setFocusToWindow = false);
    static KSError sendEnterFast(bool setFocusToWindow = false);
    static KSError sendStringFast(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false);
    static KSError sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated = true, bool setFocusToWindow = false);

    // Static async methods: spawn a thread to do this, so we don't have to pause the current thread
    static KSError sendCharFastAsync(const char ch, bool setFocusToWindow = false);
    static KSError sendEnterFastAsync(bool setFocusToWindow = false);
    static KSError sendStringFastAsync(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false);
    static KSError sendStringsFastAsync(const std::vector<std::string>& strings, bool enterTerminated = true, bool setFocusToWindow = false);

protected:
    bool m_setFocusToWindow;
    KSError m_error = KSError::Ok;
    UOClientType m_clientType = UOClientType::Unknown;
};


}

#endif // defined(__unix__) && defined(__APPLE__)

#endif // KEYSTROKESENDER_MAC_H
