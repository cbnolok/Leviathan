#ifndef KeystrokeSender_H
#define KeystrokeSender_H

#if defined(__unix__) && !defined(__APPLE__)

#include "keystrokesenderbase.h"


// forward declare Xlib types: we don't want to include Xlib.h here because it would pollute every
//  file including this header with tons of Xlib preprocessor macros. This would cause compilation errors
//  (conflict with Qt enum member names).
struct _XDisplay;
typedef _XDisplay 		Display;
typedef unsigned long	Window;

namespace ks
{


class KeystrokeSender : public KeystrokeSenderBase
{
public:
    KeystrokeSender(std::string windowTitleFragment); // set the focus to the window to which i have sent the text
    ~KeystrokeSender();

    virtual bool canSend() override;

    virtual bool sendChar(unsigned int ch, bool setFocusToWindow = false) override;
    virtual bool sendEnter(bool setFocusToWindow = false) override;
    virtual bool sendString(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false) override;

private:
    Display*   	m_display;
    Window      m_rootWindow;
    Window      m_UOWindow;

    bool findUOWindow();
    bool attach();
    void detach();

    bool _sendChar(unsigned int ch);
    bool _sendEnter();
    bool _sendString(const std::string& str, bool enterTerminated);
};


}

#endif // defined(__unix__) && !defined(__APPLE__)

#endif // KeystrokeSender_H
