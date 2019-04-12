#ifndef KEYSTROKESENDER_WINDOWS_H
#define KEYSTROKESENDER_WINDOWS_H

#if BUILD_WINDOWS

#include "keystrokesender_common.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace ks
{


class KeystrokeSender_Windows
{
    friend BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);

protected:
    KeystrokeSender_Windows(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text

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

private:
    HWND m_UOHandle = nullptr;

    bool findUOWindow();
};


}

#endif // _WIN32

#endif // KEYSTROKESENDER_WINDOWS_H
