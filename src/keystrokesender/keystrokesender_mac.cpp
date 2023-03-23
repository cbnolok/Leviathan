#if defined(__APPLE__)

#include "keystrokesender_mac.h"
#include <Carbon/Carbon.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <QString>

namespace ks
{

// static methods

CGKeyCode KeyboardCodeFromCharCode(char charCode)
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

int KeystrokeSender_Mac::findWindowPid()
{
    const QString& name = "Ultima Online";
    int Pid = 0;

    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    CFIndex numWindows = CFArrayGetCount(windowList);
    CFStringRef nameRef = name.toCFString();

    for(int i = 0; i < (int)numWindows; i++) {
        CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFStringRef thisWindowName = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
            if(thisWindowName && CFStringFind(thisWindowName, nameRef, 0).location != kCFNotFound) {
                CFNumberRef thisPidNumber = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
                CFNumberGetValue(thisPidNumber, kCFNumberIntType, &Pid);
                break;
            }
    }

    CFRelease(nameRef);
    CFRelease(windowList);
    return Pid;
}

std::string KeystrokeSender_Mac::exec(const char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

void KeystrokeSender_Mac::focusWindow(int pid)
{
    std::string cmd = "osascript -e 'tell application \"System Events\" to set frontmost of the first process whose unix id is "+std::to_string(pid)+" to true'";

    exec(&cmd[0]);
}

void KeystrokeSender_Mac::setClipboard(std::string text)
{
    std::stringstream cmd;
    cmd << "printf \"" << text << "\" | pbcopy";
    exec(cmd.str().c_str());
}


// --


KeystrokeSender_Mac::KeystrokeSender_Mac(bool setFocusToWindow) :
    m_setFocusToWindow(setFocusToWindow), m_error(KSError::Ok), m_clientType(UOClientType::Unknown)
{
}

std::string KeystrokeSender_Mac::getWindowNameThirdPartyFragment() const
{
    return m_windowNameThirdpartyFragment;
}

bool KeystrokeSender_Mac::sendChar(const char ch)
{
    sendCharFast(ch);
    return true;
}

bool KeystrokeSender_Mac::sendEnter()
{
    sendEnterFast();
    return true;
}

bool KeystrokeSender_Mac::sendString(const std::string& str, bool enterTerminated)
{
    sendStringFast(str,enterTerminated,false);
    return true;
}

bool KeystrokeSender_Mac::sendStrings(const std::vector<std::string>& strings, bool enterTerminated)
{
    sendStringsFast(strings,enterTerminated,false);
    return true;
}

/* static functions */

KSError KeystrokeSender_Mac::sendCharFast(const char ch, bool setFocusToWindow)
{
    int pid = findWindowPid();
    if (pid ==0)
        return KSError::NoWindow;
    CGKeyCode key = KeyboardCodeFromCharCode(ch);

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);

    CGEventRef keyDown = CGEventCreateKeyboardEvent(source, key, TRUE);
    CGEventRef keyUp = CGEventCreateKeyboardEvent(source, key, FALSE);

    CGEventPostToPid(pid,keyDown);
    CGEventPostToPid(pid, keyUp);

    CFRelease(keyUp);
    CFRelease(keyDown);
    CFRelease(source);
}

KSError KeystrokeSender_Mac::sendEnterFast(bool setFocusToWindow)
{
    int pid = findWindowPid();
    if (pid ==0)
        return KSError::NoWindow;
    CGEventFlags flags_shift = kCGEventFlagMaskControl;
    CGEventRef ev;
    CGEventSourceRef source = CGEventSourceCreate (kCGEventSourceStateCombinedSessionState);
    //press down
    ev = CGEventCreateKeyboardEvent (source, kVK_Return, true);
    CGEventSetFlags(ev,flags_shift | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(pid,ev);
    CFRelease(ev);

    //press up
    ev = CGEventCreateKeyboardEvent (source, kVK_Return, false);
    CGEventSetFlags(ev,flags_shift | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(pid,ev);
    CFRelease(ev);
    return KSError::Ok;
}

KSError KeystrokeSender_Mac::sendStringFast(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    int pid = findWindowPid();

    if (pid ==0)
        return KSError::NoWindow;

    //bring to front
    if (setFocusToWindow)
        focusWindow(pid);

    //add cmd to clipboard
    setClipboard(str);


    CGEventFlags flags_cmd = kCGEventFlagMaskCommand;
    CGEventRef ev;
    CGEventRef evc;
    CGEventSourceRef source = CGEventSourceCreate (kCGEventSourceStateCombinedSessionState);

    //press down ctrl
    evc = CGEventCreateKeyboardEvent (source, kVK_Command, true);
    CGEventSetFlags(evc,flags_cmd | CGEventGetFlags(evc)); //combine flags
    CGEventPostToPid(pid,evc);
    CFRelease(evc);

    std::this_thread::sleep_for(std::chrono::milliseconds(25));

    //press down
    ev = CGEventCreateKeyboardEvent (source, kVK_ANSI_V, true);
    CGEventSetFlags(ev,flags_cmd | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(pid,ev);
    CFRelease(ev);

    //press up
    ev = CGEventCreateKeyboardEvent (source, kVK_ANSI_V, false);
    CGEventSetFlags(ev,flags_cmd | CGEventGetFlags(ev)); //combine flags
    CGEventPostToPid(pid,ev);
    CFRelease(ev);

    //press up ctrl
    evc = CGEventCreateKeyboardEvent (source, kVK_Command, false);
    CGEventSetFlags(evc,CGEventGetFlags(evc)); //combine flags
    CGEventPostToPid(pid,evc);
    CFRelease(evc);

    if(enterTerminated)
    {
        sendEnterFast(setFocusToWindow);
    }

    CFRelease(source);
    return KSError::Ok;
}

KSError KeystrokeSender_Mac::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, bool setFocusToWindow)
{
    int pid = findWindowPid();

    if (pid ==0)
        return KSError::NoWindow;

    //bring to front
    if (setFocusToWindow)
    {
        focusWindow(pid);
    }

    for(auto s:strings)
    {
        sendStringFast(s,enterTerminated,false);
    }
    return KSError::Ok;
}

/* Static async functions */

KSError KeystrokeSender_Mac::sendCharFastAsync(const char ch, bool setFocusToWindow)
{
    return sendCharFast(ch,setFocusToWindow);
}

KSError KeystrokeSender_Mac::sendEnterFastAsync(bool setFocusToWindow)
{
    return sendEnterFast(setFocusToWindow);
}

KSError KeystrokeSender_Mac::sendStringFastAsync(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    return sendStringFast(str,enterTerminated,setFocusToWindow);
}

KSError KeystrokeSender_Mac::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, bool setFocusToWindow)
{
    return sendStringsFast(strings,enterTerminated,setFocusToWindow);
}

}

#endif // defined (__APPLE__)
