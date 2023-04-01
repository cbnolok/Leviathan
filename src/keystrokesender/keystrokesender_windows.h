#ifndef KeystrokeSender_H
#define KeystrokeSender_H

#ifdef _WIN32

#include "keystrokesenderbase.h"


namespace ks
{


class KeystrokeSender : public KeystrokeSenderBase
{
public:
    KeystrokeSender(std::string windowTitleFragment); // set the focus to the window to which i have sent the text
    ~KeystrokeSender() = default;

public:
    //virtual void resetWindow() override;
    virtual bool canSend() override;

    virtual bool sendChar(unsigned int ch, bool setFocusToWindow = false) override;
    virtual bool sendEnter(bool setFocusToWindow = false) override;
    virtual bool sendString(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false) override;

private:
    void* m_UOHandle;

    bool findUOWindow();
};


}

#endif // _WIN32

#endif // KeystrokeSender_H
