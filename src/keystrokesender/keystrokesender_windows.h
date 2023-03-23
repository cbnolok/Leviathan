#ifndef KEYSTROKESENDER_WINDOWS_H
#define KEYSTROKESENDER_WINDOWS_H

#ifdef _WIN32

#include "keystrokesender_common.h"


namespace ks
{


class KeystrokeSender_Windows
{
protected:
    KeystrokeSender_Windows(std::string windowTitleFragment, bool setFocusToWindow = false); // set the focus to the window to which i have sent the text
    ~KeystrokeSender_Windows() = default;

public:
    std::string getWindowNameThirdPartyFragment() const;
    bool canSend();

    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const std::string& str, bool enterTerminated = true);
    bool sendStrings(const std::vector<std::string> &strings, bool enterTerminated = true);

    // Static methods: do the work without creating an instance of the class and setting everything manually
    static KSError sendCharFast(const char ch, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendEnterFast(std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringFast(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);

    // Static async methods: spawn a thread to do this, so we don't have to pause the current thread
    static KSError sendCharFastAsync(const char ch, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendEnterFastAsync(std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringFastAsync(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringsFastAsync(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);

protected:
    bool m_setFocusToWindow;
    KSError m_error;
    UOClientType m_clientType;
    std::string m_windowNameThirdpartyFragment;

private:
    void* m_UOHandle;

    bool findUOWindow();
};


}

#endif // _WIN32

#endif // KEYSTROKESENDER_WINDOWS_H
