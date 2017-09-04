#ifndef KEYSTROKESENDER_WINDOWS_H
#define KEYSTROKESENDER_WINDOWS_H

#ifdef _WIN32

#include "keystrokesender_common.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace keystrokesender
{


class KeystrokeSender_Windows
{
public:
    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char * const str, bool enterTerminated = true);

protected:
    KSError m_error = KSERR_OK;
    UOClientType m_clientType = CLIENT_UNK;

private:
    long* m_UOHandle = 0;

    static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);
    bool findUOWindow();
    bool canSend();
};


}

#endif // _WIN32

#endif // KEYSTROKESENDER_WINDOWS_H
