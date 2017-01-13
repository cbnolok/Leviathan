#include "uoclientcom.h"
#include <string>
#include <thread>
#include <chrono>


const char *UOClientCom::UOClientComErrorString[]
{   // The string order must match the UOClientComError enum order.
    "No error.",
    "UO Client window was not found."
};

const char *UOClientCom::UOClientWindowTitles[]
{   // The string order must match the UOClientType enum order.
    "Ultima Online -",      // for Classic Client
    "UOSA -"                // for Enhanced Client
};


//UOClientCom::UOClientCom()
//{
//}

const char *UOClientCom::getErrorString()
{
    return UOClientComErrorString[m_error];
}

#ifdef _WIN32
BOOL CALLBACK UOClientCom::enumWindowsProc(HWND hWnd, LPARAM lParam)
{
    UOClientCom *classInstance = reinterpret_cast<UOClientCom*>(lParam);
    int length = GetWindowTextLengthA(hWnd);
    std::string windowTitle(length+1, '\0');
    length = GetWindowTextA(hWnd, &windowTitle[0], length);
    windowTitle.resize(length);
    if (windowTitle.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_CLASSIC;
        classInstance->m_uoHandle = (long*)hWnd;
        return FALSE;
    }
    else if (windowTitle.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_ENHANCED;
        classInstance->m_uoHandle = (long*)hWnd;
        return FALSE;
    }
    return TRUE;
}
#endif

bool UOClientCom::findUOWindow()
{
#ifdef _WIN32
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
    if (m_uoHandle == nullptr)
    {
        m_error = UOCOMERR_NOWINDOW;
        return false;
    }
    m_error = UOCOMERR_OK;
    return true;
#endif
}

bool UOClientCom::canSend()
{
    if (m_uoHandle == nullptr)
    {
        if (!findUOWindow())
            return false;
    }
    if (IsWindow((HWND)m_uoHandle))
    {
        std::string windowName;
        windowName.resize(101);
#ifdef _WIN32
        GetWindowTextA((HWND)m_uoHandle, &windowName[0], 100);
#endif
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

bool UOClientCom::sendChar(const char ch)
{
    if (!canSend())
        return false;

#ifdef _WIN32
    PostMessage((HWND)m_uoHandle, WM_CHAR, ch, 0);
#endif
return true;
}

bool UOClientCom::sendEnter()
{
    if (!canSend())
        return false;

#ifdef _WIN32
    // Also the WM_CHAR works for the enter

    // KeyDown
    PostMessage((HWND)m_uoHandle, WM_KEYDOWN, VK_RETURN, (LPARAM)( 1 ));
    // KeyUp
    PostMessage((HWND)m_uoHandle, WM_KEYUP, VK_RETURN, (LPARAM)( 1 | (1 << 30) | (1 << 31) ));
#endif
return true;
}

bool UOClientCom::sendString(const char *string)
{
    if ( (strlen(string) < 1) )
        return false;

    int len = (strlen(string) > 255) ? 255 : strlen(string);
#ifdef _WIN32
    for (int i = 0; i < len; i++)
    {
        if (!sendChar(string[i]))
            return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    if (!sendEnter())
        return false;
#endif
    return true;
}
