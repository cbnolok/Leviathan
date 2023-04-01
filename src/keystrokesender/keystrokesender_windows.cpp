#ifdef _WIN32

#include "keystrokesender_windows.h"
#include "../cpputils/utf8/unchecked.h"
#include "../cpputils/strings.h"
#include <thread>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


static constexpr int kDelayKeystrokes = 45; // milliseconds. Anything less will fail to send some characters of the string
static constexpr unsigned int kMaxStringChars = 255;


namespace ks
{

static HWND gs_windowHandleHelper = nullptr;
static std::string getWindowStdStringTitle(HWND hWnd)
{
    int length = GetWindowTextLengthW(hWnd);
    std::wstring windowTitle_wide(length + 1, '\0');
    length = GetWindowTextW(hWnd, windowTitle_wide.data(), length);
    windowTitle_wide.resize(length);

    return wideStringToString(windowTitle_wide);
}

static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
    gs_windowHandleHelper = nullptr;
    const std::string windowTitle(getWindowStdStringTitle(hWnd));
    auto *classInstance = reinterpret_cast<KeystrokeSender*>(lParam);

    const UOClientType clitype = detectClientTypeFromTitle(windowTitle, classInstance->getWindowTitleThirdpartyFragment());
    if (clitype == UOClientType::Unknown)
        return TRUE; // continue enumeration

    gs_windowHandleHelper = hWnd;
    return FALSE; // stop enumeration
}


//--


KeystrokeSender::KeystrokeSender(std::string windowTitleFragment) :
    KeystrokeSenderBase(windowTitleFragment)
{
}


bool KeystrokeSender::findUOWindow()
{
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
    m_UOHandle = gs_windowHandleHelper;
    gs_windowHandleHelper = nullptr;

    if (m_UOHandle == nullptr)
    {
        m_error = KSError::NoWindow;
        m_clientType = UOClientType::Unknown;
        return false;
    }

    m_error = KSError::Ok;
    m_clientType = detectClientTypeFromTitle(
                getWindowStdStringTitle(static_cast<HWND>(m_UOHandle)),
                getWindowTitleThirdpartyFragment() );
    return true;
}


// virtuals

/*
void KeystrokeSender::resetWindow()
{
    m_UOHandle = nullptr;
    m_clientType = UOClientType::Unknown;
    m_error = KSError::Ok;
}
*/

bool KeystrokeSender::canSend()
{
    if (m_UOHandle == nullptr)
    {
        return findUOWindow();
    }
    else if (IsWindow(static_cast<HWND>(m_UOHandle)))
    {
        EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
        const std::string strClientWindowTitle(getWindowStdStringTitle(gs_windowHandleHelper));
        gs_windowHandleHelper = nullptr;

        const UOClientType clitype = detectClientTypeFromTitle(
                    strClientWindowTitle,
                    getWindowTitleThirdpartyFragment() );
        if (clitype == m_clientType)
            return true;
        if (findUOWindow())
            return true;
    }

    m_error = KSError::NoWindow;
    return false;
}

/*
WM_CHAR message:
Bits 	Meaning
---------------
0-15 	The repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.
16-23 	The scan code. The value depends on the OEM.
24      Indicates whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
25-28 	Reserved; do not use.
29      The context code. The value is 1 if the ALT key is held down while the key is pressed; otherwise, the value is 0.
30      The previous key state. The value is 1 if the key is down before the message is sent, or it is 0 if the key is up.
31      The transition state. The value is 1 if the key is being released, or it is 0 if the key is being pressed.
*/

/* Hook for getting received messages from a window:
ClientWindowProc = (WNDPROC)SetWindowLongPtr((HWND)sdlWindowHwnd, GWLP_WNDPROC, (LONG_PTR)HookWindowProc);
LRESULT CALLBACK HookWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    qDebug() << QString::asprintf("HookWindowProc: 0x%08X, 0x%08X)", msg, wparam, lparam);
    return CallWindowProc(ClientWindowProc, hwnd, msg, wparam, lparam);
}
*/

bool KeystrokeSender::sendChar(unsigned int ch, bool setFocusToWindow)
{
    if (!canSend())
        return false;

    if (setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    //qDebug() << QString::fromUtf8(reinterpret_cast<char*>(&ch), 4);
    return PostMessageW(static_cast<HWND>(m_UOHandle), WM_CHAR, ch, 0);
}

bool KeystrokeSender::sendEnter(bool setFocusToWindow)
{
    if (!canSend())
        return false;

    if (setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    // We need to send also the key scancode, otherwise Orion client won't work. SDL/IBM PC XT Enter scancode: 1C (press), 9C (release).
    // qDebug() << Qt::hex << Qt::showbase << ( 1u | (0x1C << 16) | (1 << 30) | (1 << 31) );
    bool ok = false;

    // KeyDown
    ok = PostMessageW(static_cast<HWND>(m_UOHandle), WM_KEYDOWN, VK_RETURN, (LPARAM)( 1u | (0x1C << 16)) );
    if (!ok)
        return false;

    std::this_thread::sleep_for(std::chrono::milliseconds(kDelayKeystrokes / 2));

    // KeyUp
    ok = PostMessageW(static_cast<HWND>(m_UOHandle), WM_KEYUP, VK_RETURN, (LPARAM)( 1u | (0x1C << 16) | (1 << 30) | (1 << 31) ));

    return ok;
}


bool KeystrokeSender::sendString(const std::string &str, bool enterTerminated, bool setFocusToWindow)
{
    if ( str.length() < 1 )
    {
        m_error = KSError::StringShort;
        return false;
    }
    if (!canSend())
        return false;

    // In the past (maybe because of how older versions of the Enhanced Client were coded), PostMessage didn't work
    //  for EC but only for the Classic Client, so we needed to use SendInput, which required the receiver window to
    //  be the foreground window.

    if (setFocusToWindow)
        SetForegroundWindow(static_cast<HWND>(m_UOHandle));

    size_t iSentUTF8Chars = 0;
    for (const char* it = str.data(); (*it != '\0') && (iSentUTF8Chars <= kMaxStringChars); ++iSentUTF8Chars)
    {
        const unsigned int ch = utf8::unchecked::next(it);
        if (!sendChar(ch, false))
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


}

#endif
