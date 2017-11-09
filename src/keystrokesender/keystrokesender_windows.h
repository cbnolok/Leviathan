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
    bool canSend();

    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char * const str, bool enterTerminated = true);

protected:
    KSError m_error = KSERR_OK;
    UOClientType m_clientType = CLIENT_UNK;

private:
    long* m_UOHandle = 0;

    bool findUOWindow();
};


}

#endif // _WIN32

#endif // KEYSTROKESENDER_WINDOWS_H
