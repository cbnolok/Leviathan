#ifndef KEYSTROKESENDER_LINUX_H
#define KEYSTROKESENDER_LINUX_H

#if defined(__unix__) && !defined(__APPLE__)

#include "keystrokesender_common.h"


// forward declare Xlib types: we don't want to include Xlib.h here because it would pollute every
//  file including this header with tons of Xlib preprocessor macros. This would cause compilation errors
//  (conflict with Qt enum member names).
struct _XDisplay;
typedef _XDisplay 		Display;
typedef unsigned long	Window;

namespace ks
{


class KeystrokeSender_Linux
{
protected:
    KeystrokeSender_Linux(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text
    ~KeystrokeSender_Linux();

public:
    std::string getWindowNameThirdPartyFragment() const;
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
    KSError m_error;
    UOClientType m_clientType;
    std::string m_windowNameThirdpartyFragment;

private:
    Display*   	m_display;
    Window      m_rootWindow;
    Window      m_UOWindow;

    bool findUOWindow();
    bool attach();
    void detach();
    bool _sendChar(const char ch);
    bool _sendEnter();
    bool _sendString(const std::string& str, bool enterTerminated = true);
};


}

#endif // defined(__unix__) && !defined(__APPLE__)

#endif // KEYSTROKESENDER_LINUX_H
