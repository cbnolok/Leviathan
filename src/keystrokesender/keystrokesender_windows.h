#ifndef KEYSTROKESENDER_WINDOWS_H
#define KEYSTROKESENDER_WINDOWS_H

#ifdef _WIN32

#include "keystrokesender_common.h"
#include <windef.h>     // for the definitions of BOOL, CALLBACK, HWND, LPARAM


namespace keystrokesender
{


class KeystrokeSender_Windows
{
    friend BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);

public:
    KeystrokeSender_Windows(bool setFocusToWindow = false); // set the focus to the window to which i have sent the text

    bool canSend();

    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char * const str, bool enterTerminated = true);

protected:
    bool m_setFocusToWindow;
    KSError m_error = KSERR_OK;
    UOClientType m_clientType = CLIENT_UNK;

private:
    HWND m_UOHandle = 0;

    bool findUOWindow();
};


}

#endif // _WIN32

#endif // KEYSTROKESENDER_WINDOWS_H
