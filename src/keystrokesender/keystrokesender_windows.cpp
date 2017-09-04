#ifdef _WIN32

#include "keystrokesender_windows.h"
#include <string>
#include <thread>
#include <chrono>

#include <winuser.h>


namespace keystrokesender
{


BOOL CALLBACK KeystrokeSender_Windows::enumWindowsProc(HWND hWnd, LPARAM lParam)
{
    KeystrokeSender_Windows *classInstance = reinterpret_cast<KeystrokeSender_Windows*>(lParam);
    int length = GetWindowTextLengthA(hWnd);
    std::string windowTitle(length+1, '\0');
    length = GetWindowTextA(hWnd, &windowTitle[0], length);
    windowTitle.resize(length);
    if (windowTitle.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_CLASSIC;
        classInstance->m_UOHandle = reinterpret_cast<long*>(hWnd);
        return FALSE;
    }
    else if (windowTitle.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_ENHANCED;
        classInstance->m_UOHandle = reinterpret_cast<long*>(hWnd);
        return FALSE;
    }
    return TRUE;
}

bool KeystrokeSender_Windows::findUOWindow()
{
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
    if (m_UOHandle == nullptr)
    {
        m_error = KSERR_NOWINDOW;
        return false;
    }
    m_error = KSERR_OK;
    return true;
}

bool KeystrokeSender_Windows::canSend()
{
    if (m_UOHandle == nullptr)
    {
        if (!findUOWindow())
            return false;
    }
    if (IsWindow(reinterpret_cast<HWND>(m_UOHandle)))
    {
        std::string windowName;
        windowName.resize(101);
        GetWindowTextA(reinterpret_cast<HWND>(m_UOHandle), &windowName[0], 100);
        if ( (m_clientType == CLIENT_CLASSIC) && (windowName.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos) )
            return true;
        else if ( (m_clientType == CLIENT_ENHANCED) && (windowName.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos) )
            return true;
        else if (findUOWindow())
            return true;
        else
            return false;
    }
    else
        return false;
    return true;
}

bool KeystrokeSender_Windows::sendChar(const char ch)
{
    if (!canSend())
        return false;

    PostMessage(reinterpret_cast<HWND>(m_UOHandle), WM_CHAR, ch, 0);
    return true;
}

bool KeystrokeSender_Windows::sendEnter()
{
    if (!canSend())
        return false;

    // Also the WM_CHAR works for the enter key

    // KeyDown
    PostMessage(reinterpret_cast<HWND>(m_UOHandle), WM_KEYDOWN, VK_RETURN, (LPARAM)( 1 ));

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // KeyUp
    PostMessage(reinterpret_cast<HWND>(m_UOHandle), WM_KEYUP, VK_RETURN, (LPARAM)( 1 | (1 << 30) | (1 << 31) ));

    return true;
}

bool KeystrokeSender_Windows::sendString(const char * const str, bool enterTerminated)
{
    if ( (strlen(str) < 1) )
    {
        m_error = KSERR_STRINGSHORT;
        return false;
    }

// TODO: cansend(); so i can detect the client type
// TODO: if Enhanched Client (or KR? or UO3D?), for windows use SendInput instead of PostMessage and

    int len = (strlen(str) > 255) ? 255 : (int)strlen(str);

    for (int i = 0; i < len; ++i)
    {
        if (!sendChar(str[i]))
            return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    if (enterTerminated)
    {
        if (!sendEnter())
            return false;
    }

    return true;
}


}

#endif
