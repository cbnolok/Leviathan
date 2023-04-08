#ifndef KEYSTROKESENDER_MAC_H
#define KEYSTROKESENDER_MAC_H

#if defined(__APPLE__)

#include "keystrokesenderbase.h"

namespace ks
{

class KeystrokeSender : public KeystrokeSenderBase
{
public:
    KeystrokeSender(std::string windowTitleFragment); // set the focus to the window to which i have sent the text
    ~KeystrokeSender() = default;

protected:
    bool findUOWindow(const char* windowName);

public:
    virtual bool canSend() override;

    virtual bool sendChar(unsigned int ch, bool setFocusToWindow = false) override;
    virtual bool sendEnter(bool setFocusToWindow = false) override;
    virtual bool sendString(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false) override;

protected:
    int m_UOPid;

    bool findUOWindow();

    void _sendChar(unsigned int ch);
    void _sendEnter();
    void _pasteString(const std::string& str);
};


}

#endif // defined(__APPLE__)

#endif // KEYSTROKESENDER_MAC_H
