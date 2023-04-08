#if defined(__APPLE__)

#include "KeystrokeSender.h"
#include <Carbon/Carbon.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <memory>


static CFStringRef cStringToCFString(const char* inStr)
{
    return CFStringCreateWithCString(NULL, inStr, kCFStringEncodingUTF8);
}

static CFStringRef stdStringToCFString(std::string const& inStr)
{
    return CFStringCreateWithCString(NULL, inStr.c_str(), kCFStringEncodingUTF8);
}

static std::unique_ptr<char[]> cUTF8StringFromCFString(CFStringRef aString)
{
    if (aString == NULL)
        return NULL;

    const CFIndex length = CFStringGetLength(aString);
    const CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    auto strNew = std::make_unique<char[]>(maxSize);

    if (!CFStringGetCString(aString, strNew.get(), maxSize, kCFStringEncodingUTF8))
        strNew.reset();

    return strNew;
}


namespace ks
{


static CGKeyCode keyboardCodeFromCharCode(char charCode)
{
    switch (charCode)
    {
        case 'a': case 'A': return kVK_ANSI_A;
        case 'b': case 'B': return kVK_ANSI_B;
        case 'c': case 'C': return kVK_ANSI_C;
        case 'd': case 'D': return kVK_ANSI_D;
        case 'e': case 'E': return kVK_ANSI_E;
        case 'f': case 'F': return kVK_ANSI_F;
        case 'g': case 'G': return kVK_ANSI_G;
        case 'h': case 'H': return kVK_ANSI_H;
        case 'i': case 'I': return kVK_ANSI_I;
        case 'j': case 'J': return kVK_ANSI_J;
        case 'k': case 'K': return kVK_ANSI_K;
        case 'l': case 'L': return kVK_ANSI_L;
        case 'm': case 'M': return kVK_ANSI_M;
        case 'n': case 'N': return kVK_ANSI_N;
        case 'o': case 'O': return kVK_ANSI_O;
        case 'p': case 'P': return kVK_ANSI_P;
        case 'q': case 'Q': return kVK_ANSI_Q;
        case 'r': case 'R': return kVK_ANSI_R;
        case 's': case 'S': return kVK_ANSI_S;
        case 't': case 'T': return kVK_ANSI_T;
        case 'u': case 'U': return kVK_ANSI_U;
        case 'v': case 'V': return kVK_ANSI_V;
        case 'w': case 'W': return kVK_ANSI_W;
        case 'x': case 'X': return kVK_ANSI_X;
        case 'y': case 'Y': return kVK_ANSI_Y;
        case 'z': case 'Z': return kVK_ANSI_Z;
        case '-': case '_': return kVK_ANSI_Minus;
        case '.': case '>': return kVK_ANSI_Period;
        case ' ': return kVK_Space;
  }
  return 0;
}

static std::string pexec(const char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
        return "ERROR";
    char buffer[128];
    std::string result;
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

static void focusWindow(int pid)
{
    std::string cmd = "osascript -e 'tell application \"System Events\" to set frontmost of the first process whose unix id is " + std::to_string(pid) + " to true'";
    pexec(cmd.c_str());
}

static void setClipboard(std::string const& text)
{
    std::stringstream cmd;
    cmd << "printf \"" << text << "\" | pbcopy";
    pexec(cmd.str().c_str());
}



// --


KeystrokeSender::KeystrokeSender(std::string windowTitleFragment) :
    KeystrokeSenderBase(windowTitleFragment),
    m_UOPid(0)
{
}


bool KeystrokeSender::findUOWindow()
{
    m_UOPid = 0;
    m_clientType = UOClientType::Unknown;

    //CFStringRef windowNameRef = cStringToCFString(windowName);
    int Pid = 0;

    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    const CFIndex numWindows = CFArrayGetCount(windowList);

    for (CFIndex i = 0; i < numWindows; ++i)
    {
        CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFStringRef thisWindowName = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
        std::unique_ptr<char[]> thisWindowNameCStr = cUTF8StringFromCFString(thisWindowName);
        UOClientType clitype = detectClientTypeFromTitle(thisWindowNameCStr.get(), getWindowTitleThirdpartyFragment());

        //if(thisWindowName && CFStringFind(thisWindowName, nameRef, 0).location != kCFNotFound)
        if (clitype != UOClientType::Unknown)
        {
            CFNumberRef thisPidNumber = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
            CFNumberGetValue(thisPidNumber, kCFNumberIntType, &Pid);

            m_UOPid = Pid;
            m_clientType = clitype;
            break;
        }
    }

    //CFRelease(windowNameRef);
    CFRelease(windowList);

    return (0 != Pid);
}

bool KeystrokeSender::canSend()
{
    if (m_UOPid == 0)
    {
        return findUOWindow();
    }
    return true;
}

void KeystrokeSender::_sendChar(unsigned int ch)
{
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);

    const UniChar uch = std::max((unsigned int)UINT16_MAX, ch);
    CGEventRef keyDown = CGEventCreateKeyboardEvent(source, 0, TRUE);
    CGEventRef keyUp = CGEventCreateKeyboardEvent(source, 0, FALSE);
    CGEventKeyboardSetUnicodeString(keyDown, 1, &uch);
    CGEventKeyboardSetUnicodeString(keyUp, 1, &uch);

    CGEventPostToPid(m_UOPid, keyDown);
    CGEventPostToPid(m_UOPid, keyUp);

    CFRelease(keyUp);
    CFRelease(keyDown);
    CFRelease(source);
}

bool KeystrokeSender::sendChar(unsigned int ch, bool setFocusToWindow)
{
    if (!canSend())
        return false;

    if (setFocusToWindow)
        focusWindow(m_UOPid);

    _sendChar(ch);

    return true;
}

void KeystrokeSender::_sendEnter()
{
    const CGEventFlags flags_shift = kCGEventFlagMaskControl;
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    CGEventRef ev;

    //CGEventSetType(keyEvent, kCGEventKeyUp);
    //CGEventSetType(keyEvent, kCGEventKeyDown);

    // press down
    ev = CGEventCreateKeyboardEvent (source, kVK_Return, true);
    CGEventSetFlags(ev, flags_shift | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(m_UOPid, ev);
    CFRelease(ev);

    // press up
    ev = CGEventCreateKeyboardEvent (source, kVK_Return, false);
    CGEventSetFlags(ev, flags_shift | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(m_UOPid, ev);
    CFRelease(ev);

    CFRelease(source);
}

bool KeystrokeSender::sendEnter(bool setFocusToWindow)
{
    if (!canSend())
        return false;

    if (setFocusToWindow)
        focusWindow(m_UOPid);

    _sendEnter();

    return true;
}

void KeystrokeSender::_pasteString(const std::string& str)
{
    //add cmd to clipboard
    setClipboard(str);

    const CGEventFlags flags_cmd = kCGEventFlagMaskCommand;
    CGEventSourceRef source = CGEventSourceCreate (kCGEventSourceStateCombinedSessionState);
    CGEventRef ev;
    CGEventRef evc;

    //press down ctrl
    evc = CGEventCreateKeyboardEvent (source, kVK_Command, true);
    CGEventSetFlags(evc,flags_cmd | CGEventGetFlags(evc)); //combine flags
    CGEventPostToPid(m_UOPid,evc);
    CFRelease(evc);

    std::this_thread::sleep_for(std::chrono::milliseconds(25));

    //press down
    ev = CGEventCreateKeyboardEvent (source, kVK_ANSI_V, true);
    CGEventSetFlags(ev,flags_cmd | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(m_UOPid,ev);
    CFRelease(ev);

    //press up
    ev = CGEventCreateKeyboardEvent (source, kVK_ANSI_V, false);
    CGEventSetFlags(ev,flags_cmd | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(m_UOPid,ev);
    CFRelease(ev);

    //press up ctrl
    evc = CGEventCreateKeyboardEvent (source, kVK_Command, false);
    CGEventSetFlags(evc,CGEventGetFlags(evc)); //combine flags
    CGEventPostToPid(m_UOPid,evc);
    CFRelease(evc);

    CFRelease(source);
}

bool KeystrokeSender::sendString(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    if (!canSend())
        return false;

    //bring to front
    if (setFocusToWindow)
        focusWindow(m_UOPid);

    _pasteString(str);

    if (enterTerminated)
    {
        _sendEnter();
    }

    return true;
}


}

#endif // defined (__APPLE__)
