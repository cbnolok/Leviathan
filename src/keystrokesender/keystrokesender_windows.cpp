#ifdef _WIN32

#include "keystrokesender_windows.h"
#include "../cpputils/strings.h"
#include <thread>
#include <chrono>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


static constexpr int kDelayKeystrokes = 50; //milliseconds
static HWND windowHandleHelper = nullptr;

namespace ks
{

static std::string getWindowStdStringTitle(HWND hWnd)
{
    int length = GetWindowTextLengthW(hWnd);
    std::wstring windowTitle_wide(length + 1, '\0');
    length = GetWindowTextW(hWnd, &windowTitle_wide[0], length);
    windowTitle_wide.resize(length);

    return wideStringToString(windowTitle_wide);
}

static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
    windowHandleHelper = nullptr;
    const std::string windowTitle(getWindowStdStringTitle(hWnd));
    KeystrokeSender_Windows *classInstance = reinterpret_cast<KeystrokeSender_Windows*>(lParam);

    UOClientType clitype = detectClientType(windowTitle, classInstance->getWindowNameThirdPartyFragment());
    if (clitype == UOClientType::Unknown)
        return FALSE;

    windowHandleHelper = hWnd;
    return TRUE;
}


KeystrokeSender_Windows::KeystrokeSender_Windows(std::string windowTitleFragment, bool setFocusToWindow) :
    m_setFocusToWindow(setFocusToWindow), m_error(KSError::Ok), m_clientType(UOClientType::Unknown),
    m_windowNameThirdpartyFragment(windowTitleFragment),
    m_UOHandle(nullptr)
{
}


std::string KeystrokeSender_Windows::getWindowNameThirdPartyFragment() const
{
    return m_windowNameThirdpartyFragment;
}

bool KeystrokeSender_Windows::findUOWindow()
{
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
    m_UOHandle = windowHandleHelper;
    windowHandleHelper = nullptr;

    if (m_UOHandle == nullptr)
    {
        m_error = KSError::NoWindow;
        return false;
    }
    m_error = KSError::Ok;
    return true;
}

bool KeystrokeSender_Windows::canSend()
{
    if (m_UOHandle == nullptr)
    {
        if (!findUOWindow())
            return false;
    }
    if (IsWindow(static_cast<HWND>(m_UOHandle)))
    {
        EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
        const std::string windowName(getWindowStdStringTitle(windowHandleHelper));
        windowHandleHelper = nullptr;

        const UOClientType clitype = detectClientType(windowName, getWindowNameThirdPartyFragment());
        if (clitype == m_clientType)
            return true;
        if (findUOWindow())
            return true;
        return false;
    }
    return findUOWindow();
}

bool KeystrokeSender_Windows::sendChar(const char ch)
{
    if (!canSend())
        return false;

    if (m_setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    PostMessage(static_cast<HWND>(m_UOHandle), WM_CHAR, ch, 0);
    return true;
}

bool KeystrokeSender_Windows::sendEnter()
{
    if (!canSend())
        return false;

    if (m_setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    // Also the WM_CHAR works for the enter key

    // KeyDown
    PostMessage(static_cast<HWND>(m_UOHandle), WM_KEYDOWN, VK_RETURN, (LPARAM)( 1 ));

    std::this_thread::sleep_for(std::chrono::milliseconds(kDelayKeystrokes));

    // KeyUp
    PostMessage(static_cast<HWND>(m_UOHandle), WM_KEYUP, VK_RETURN, (LPARAM)( 1 | (1 << 30) | (1u << 31) ));

    return true;
}

bool KeystrokeSender_Windows::sendString(const std::string &str, bool enterTerminated)
{
    if ( str.length() < 1 )
    {
        m_error = KSError::StringShort;
        return false;
    }

    // In the past (maybe because of how older versions of the Enhanced Client were coded), PostMessage didn't work
    //  for EC but only for the Classic Client, so we needed to use SendInput, which required the receiver window to
    //  be the foreground window.

    if (m_setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    unsigned len = (str.length() > 255) ? 255 : unsigned(str.length());

    for (unsigned i = 0; i < len; ++i)
    {
        if (!sendChar(str[i]))
            return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(kDelayKeystrokes));
    }

    if (enterTerminated)
    {
        if (!sendEnter())
            return false;
    }

    return true;
}

bool KeystrokeSender_Windows::sendStrings(const std::vector<std::string>& strings, bool enterTerminated)
{
    for (const std::string& str : strings)
    {
        if (!sendString(str, enterTerminated))
            return false;
    }

    return true;
}


/* static functions */

KSError KeystrokeSender_Windows::sendCharFast(const char ch, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(windowTitleFragment, setFocusToWindow);
    ks.sendChar(ch);
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendEnterFast(std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(windowTitleFragment, setFocusToWindow);
    ks.sendEnter();
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendStringFast(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(windowTitleFragment, setFocusToWindow);
    ks.sendString(str, enterTerminated);
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(windowTitleFragment, setFocusToWindow);
    for (const std::string& str : strings)
    {
        ks.sendString(str, enterTerminated);
        if (ks.m_error != KSError::Ok)
            return ks.m_error;
    }
    return ks.m_error;
}


/* Static async functions */

KSError KeystrokeSender_Windows::sendCharFastAsync(const char ch, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(windowTitleFragment, setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(windowTitleFragment, setFocusToWindow);
            ks_thread.sendChar(ch);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendEnterFastAsync(std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(windowTitleFragment, setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(windowTitleFragment, setFocusToWindow);
            ks_thread.sendEnter();
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendStringFastAsync(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(windowTitleFragment, setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    if ( str.length() < 1 )
        return KSError::StringShort;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(windowTitleFragment, setFocusToWindow);
            ks_thread.sendString(str, enterTerminated);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(windowTitleFragment, setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(windowTitleFragment, setFocusToWindow);
            ks_thread.sendStrings(strings, enterTerminated);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}


}

#endif
