#ifndef KEYSTROKESENDER_LINUX_H
#define KEYSTROKESENDER_LINUX_H

#ifndef _WIN32

#include "keystrokesender_common.h"


// forward declare Xlib types: we don't want to include Xlib.h here because it would pullute every
//  file including this header with tons of Xlib preprocessor macros. This would cause compilation errors
//  (conflict with Qt enum member names).
struct _XDisplay;
typedef _XDisplay 		Display;
typedef unsigned long	Window;

namespace ks
{


class KeystrokeSender_Linux
{
public:
    KeystrokeSender_Linux(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text
    ~KeystrokeSender_Linux();

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

private:
    Display*   	m_display       = 0;	// NULL
    Window      m_rootWindow    = 0;
    Window      m_UOWindow      = 0;

    bool findUOWindow();
    bool attach();
    void detach();
    bool _sendChar(const char ch);
    bool _sendEnter();
    bool _sendString(const std::string& str, bool enterTerminated = true);
};


}

#endif // !_WIN32

#endif // KEYSTROKESENDER_LINUX_H
