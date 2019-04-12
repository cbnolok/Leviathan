#if BUILD_LINUX

#include "keystrokesender_linux.h"
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

#include <X11/Xlib.h>
#include <X11/keysym.h>


// Based on:
// Send a fake keystroke event to an X window.
// by Adam Pierce - http://www.doctort.org/adam/
// This is public domain software. It is free to use by anyone for any purpose.

// And on:
// https://stackoverflow.com/questions/30156202/xlib-c-get-window-handle-sendevent


// !!! To compile this file, the following linker instructions are needed: -L/usr/X11R6/lib -lX11 !!!


namespace ks
{

KeystrokeSender_Linux::KeystrokeSender_Linux(bool setFocusToWindow) :
    m_setFocusToWindow(setFocusToWindow)
{
}

KeystrokeSender_Linux::~KeystrokeSender_Linux()
{
    detach();
}

/*  Convenience X functions */

// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &window,
                         Window &winRoot, bool press, int keycode, int modifiers)
{
    XKeyEvent event;

    event.display     = display;
    event.window      = window;
    event.root        = winRoot;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = XKeysymToKeycode(display, keycode);
    event.state       = modifiers;

    if (press)
        event.type = KeyPress;
    else
        event.type = KeyRelease;

    return event;
}

Window *getWindowList(Display *display, Window &winRoot, unsigned long &windowListSize)
{
    Atom prop = XInternAtom(display,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(display,winRoot,prop,0,1024,False,33,
                           &type,&form,&windowListSize,&remain,&list) != Success)  // XA_WINDOW
    {
        return NULL;
    }

    return (Window*)list;
}

char *getWindowName(Display *display, Window window)
{
    Atom prop = XInternAtom(display,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;


    if (XGetWindowProperty(display,window,prop,0,1024,False,AnyPropertyType,
                           &type,&form,&len,&remain,&list) != Success) // XA_STRING
    {
        return NULL;
    }

    return (char*)list;
}

// check if window sill exists
bool isWindow(Window &windowToFind, Display *display, Window &winRoot)
{
    unsigned long windowListSize;
    Window* list = getWindowList(display, winRoot, windowListSize);

    for (unsigned long i = 0; i < windowListSize; ++i)
    {
        if (list[i] == windowToFind)
            return true;
    }
    return false;
}

// from: https://stackoverflow.com/questions/2858263/how-do-i-bring-a-processes-window-to-the-foreground-on-x-windows-c
void setForegroundWindow(Display *display, Window window)   // set the desktop's topmost window
{
    XEvent event = { 0 };
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom( display, "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = window;
    event.xclient.format = 32;

    XSendEvent( display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event );
    XMapRaised( display, window );
}


/* Implementation for Leviathan */

bool KeystrokeSender_Linux::findUOWindow()
{
    // Obtain the windows list.
    unsigned long windowsCount;
    Window *list;
    list = getWindowList(m_display,m_rootWindow,windowsCount);

    // Obtain the ID of the window with the given title.
    m_UOWindow = None;
    for (unsigned long i = 0; i < windowsCount; ++i)
    {
        char* windowName = getWindowName(m_display,list[i]);
        std::string strWindowName(windowName);
        free(windowName);

        if ( strWindowName.find(UOClientWindowTitles[(int)UOClientType::Classic]) != std::string::npos )
            m_UOWindow = list[i];
        else if ( strWindowName.find(UOClientWindowTitles[(int)UOClientType::Enhanced]) != std::string::npos )
            m_UOWindow = list[i];

        if ( m_UOWindow != None )
        {
            m_error = KSError::Ok;
            return true;
        }
    }

    m_error = KSError::NoWindow;
    return false;
}

bool KeystrokeSender_Linux::attach()
{
    // Obtain the X11 display.
    m_display = XOpenDisplay(NULL);
    if (m_display == NULL)
        return false;

    // Get the root window for the current display.
    m_rootWindow = XDefaultRootWindow(m_display);

    // Find the window where to send the keystroke.
    if ( !findUOWindow() )
    {
        XCloseDisplay(m_display);
        m_display = NULL;
        return false;
    }
    return true;
}

void KeystrokeSender_Linux::detach()
{
    if (m_display != NULL)
    {
        XCloseDisplay(m_display);
        m_display = NULL;
    }
}

bool KeystrokeSender_Linux::_sendChar(const char ch)
{
    // Send a fake key press event to the window.
    XKeyEvent event = createKeyEvent(m_display, m_UOWindow, m_rootWindow, true, ch, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // Send a fake key release event to the window.
    event = createKeyEvent(m_display, m_UOWindow, m_rootWindow, false, ch, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

    return true;
}

bool KeystrokeSender_Linux::sendChar(const char ch)
{
    if (!attach())
        return false;

    if (m_setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendChar(ch);

    detach();
    return ret;
}

bool KeystrokeSender_Linux::_sendEnter()
{
    return sendChar((char)XK_Return);
}

bool KeystrokeSender_Linux::sendEnter()
{
    if (!attach())
        return false;

    if (m_setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendEnter();

    detach();
    return ret;
}

bool KeystrokeSender_Linux::_sendString(const std::string& str, bool enterTerminated)
{
    unsigned len = (str.length() > 255) ? 255 : unsigned(str.length());

    for (unsigned i = 0; i < len; ++i)
    {
        if ( !_sendChar(str[i]) )
            return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    if (enterTerminated)
    {
        if ( !_sendEnter() )
            return false;
    }

    return true;
}


bool KeystrokeSender_Linux::sendString(const std::string& str, bool enterTerminated)
{
    if ( str.length() < 1 )
    {
        m_error = KSError::StringShort;
        return false;
    }

    if (!attach())
        return false;

	if (m_setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendString(str, enterTerminated);

    detach();
    return ret;
}

bool KeystrokeSender_Linux::sendStrings(const std::vector<std::string>& strings, bool enterTerminated)
{
    if (!attach())
        return false;

    if (m_setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = true;
    for (const std::string& str : strings)
    {
        if ( str.length() < 1 )
        {
            m_error = KSError::StringShort;
            ret = false;
            break;
        }
        if (!_sendString(str, enterTerminated))
        {
            ret = false;
            break;
        }
    }

    detach();
    return ret;
}


/* static functions */

KSError KeystrokeSender_Linux::sendCharFast(const char ch, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks(setFocusToWindow);
    ks.sendChar(ch);
    return ks.m_error;
}

KSError KeystrokeSender_Linux::sendEnterFast(bool setFocusToWindow)
{
    KeystrokeSender_Linux ks(setFocusToWindow);
    ks.sendEnter();
    return ks.m_error;
}

KSError KeystrokeSender_Linux::sendStringFast(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks(setFocusToWindow);
    ks.sendString(str, enterTerminated);
    return ks.m_error;
}

KSError KeystrokeSender_Linux::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks(setFocusToWindow);
    ks.sendStrings(strings, enterTerminated);
    return ks.m_error;
}


/* Static async functions */

KSError KeystrokeSender_Linux::sendCharFastAsync(const char ch, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks_check(setFocusToWindow);
    bool check = ks_check.attach();
    ks_check.detach();
    if (!check)
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
             KeystrokeSender_Linux ks_thread(setFocusToWindow);
            ks_thread.sendChar(ch);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Linux::sendEnterFastAsync(bool setFocusToWindow)
{
    KeystrokeSender_Linux ks_check(setFocusToWindow);
    bool check = ks_check.attach();
    ks_check.detach();
    if (!check)
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
             KeystrokeSender_Linux ks_thread(setFocusToWindow);
            ks_thread.sendEnter();
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Linux::sendStringFastAsync(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks_check(setFocusToWindow);
    bool check = ks_check.attach();
    ks_check.detach();
    if (!check)
        return ks_check.m_error;

    if ( str.length() < 1 )
        return KSError::StringShort;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Linux ks_thread(setFocusToWindow);
            ks_thread.sendString(str, enterTerminated);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Linux::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Linux ks_check(setFocusToWindow);
    bool check = ks_check.attach();
    ks_check.detach();
    if (!check)
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Linux ks_thread(setFocusToWindow);
            ks_thread.sendStrings(strings, enterTerminated);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}


/*
 * Function which does all the stuff (using the X functions above). Keeping it for reference.
*/
/*
bool KeystrokeSender_Linux::sendChar(const char ch)
{
    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if (display == NULL)
        return false;

    // Get the root window for the current display.
    Window winRoot = XDefaultRootWindow(display);

    // Obtain the windows list.
    unsigned long windowsCount;
    Window *list;
    list = getWindowList(display,winRoot,windowsCount);

    // Obtain the handle of the window with the given title.
    char* windowName;
    Window* UOWindow = NULL;
    for (unsigned long i = 0; i < windowsCount; ++i)
    {
        windowName = getWindowName(display,list[i]);
        std::string strWindowName(windowName);
        free(windowName);
        //std::cout << i << ": " << strWindowName << std::endl;
        if ( strWindowName.find(UOClientWindowTitles[(int)UOClientType::Classic]) != std::string::npos )
            UOWindow = &list[i];
        else if ( strWindowName.find(UOClientWindowTitles[(int)UOClientType::Enhanced]) != std::string::npos )
            UOWindow = &list[i];
        if ( UOWindow != NULL)
            break;
    }
    std::cout << "UOWindow: " << UOWindow << std::endl;
    if (UOWindow == NULL)
        return false;

    // Or find the window which has the current keyboard focus.
    //Window winFocus;
    //int    revert;
    //XGetInputFocus(display, &winFocus, &revert);

    // these won't work with WINE windows? tbh i haven't tested them with native windows
    //XRaiseWindow(display, *UOWindow);
    //XSetInputFocus(display, *UOWindow, RevertToParent, time(NULL)+1500);

    // Send a fake key press event to the window.
    XKeyEvent event = createKeyEvent(display, *UOWindow, winRoot, true, ch, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Send a fake key release event to the window.
    event = createKeyEvent(display, *UOWindow, winRoot, false, ch, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

    //std::cout << "sent " << ch << std::endl;

    // Done.
    XCloseDisplay(display);

    return true;
}

*/


}

// undef some ugly Xlib macros...
#undef True
#undef False
#undef None

#endif // defined(__unix__) && !defined(__APPLE__)
