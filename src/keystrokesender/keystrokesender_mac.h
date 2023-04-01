#ifndef KeystrokeSender_H
#define KeystrokeSender_H

#if defined(__APPLE__)

#include "keystrokesender_common.h"

namespace ks
{

class KeystrokeSender
{
protected:
    KeystrokeSender(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text
    ~KeystrokeSender() = default;

    bool findUOWindow(const char* windowName);
    void resetWindow();

public:
    std::string getWindowNameThirdPartyFragment() const;
    bool canSend();

    bool sendChar(unsigned int  ch);
    bool sendEnter();
    bool sendString(const std::string& str, bool enterTerminated = true);
    bool sendStrings(const std::vector<std::string> &strings, bool enterTerminated = true);

    // Static methods: do the work without creating an instance of the class and setting everything manually
    static KSError sendCharFast(unsigned int  ch, bool setFocusToWindow = false);
    static KSError sendEnterFast(bool setFocusToWindow = false);
    static KSError sendStringFast(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false);
    static KSError sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated = true, bool setFocusToWindow = false);

    // Static async methods: spawn a thread to do this, so we don't have to pause the current thread
    static KSError sendCharFastAsync(unsigned int  ch, bool setFocusToWindow = false);
    static KSError sendEnterFastAsync(bool setFocusToWindow = false);
    static KSError sendStringFastAsync(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false);
    static KSError sendStringsFastAsync(const std::vector<std::string>& strings, bool enterTerminated = true, bool setFocusToWindow = false);

protected:
    bool m_setFocusToWindow;
    KSError m_error;
    UOClientType m_clientType;
    std::string m_windowNameThirdpartyFragment;

    int m_UOPid;
};


}

#endif // defined(__APPLE__)

#endif // KeystrokeSender_H
