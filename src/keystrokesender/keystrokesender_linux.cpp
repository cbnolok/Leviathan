#if defined(__unix__) && !defined(__APPLE__)

#include "keystrokesender_linux.h"
#include <cstring>
#include <string>
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

// TODO: it might not work with an official client running on WINE.
// WINE seems to handle X11 events via those functions: X11DRV_ClientMessage (dlls/winex11.drv/event.c), X11DRV_KeyEvent (dlls/winex11.drv/keyboard.c),
// X11DRV_send_keyboard_input (dlls/winex11.drv/keyboard.c) -> __wine_send_input


// !!! To compile this file, the following linker instructions are needed: -L/usr/X11R6/lib -lX11 !!!


namespace ks
{

/*  Convenience X functions */

// Function to create a keyboard event
static XKeyEvent createKeyEvent(Display *display, Window &window,
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

static Window *getWindowList(Display *display, Window &winRoot, unsigned long &windowListSize)
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

static char *getWindowName(Display *display, Window window)
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
static bool isWindow(Window &windowToFind, Display *display, Window &winRoot)
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
static void setForegroundWindow(Display *display, Window window)   // set the desktop's topmost window
{
    XEvent event = { 0 };
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = window;
    event.xclient.format = 32;

    XSendEvent( display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event );
    XMapRaised( display, window );
}



/* Implementation for Leviathan */

KeystrokeSender::KeystrokeSender(std::string windowTitleFragment) : KeystrokeSenderBase(windowTitleFragment),
    m_display(nullptr), m_rootWindow(0), m_UOWindow(0)
{
}

KeystrokeSender::~KeystrokeSender()
{
    detach();
}


bool KeystrokeSender::findUOWindow()
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
        const std::string strWindowName(windowName);
        free(windowName);

        const UOClientType clitype = detectClientTypeFromTitle(strWindowName, getWindowTitleThirdpartyFragment());
        if (clitype == UOClientType::Unknown)
            continue;

        m_UOWindow = list[i];
        m_error = KSError::Ok;
        return true;
    }

    m_error = KSError::NoWindow;
    return false;
}

bool KeystrokeSender::canSend()
{
    if (m_UOWindow == None)
    {
        return findUOWindow();
    }
    else if (isWindow(m_UOWindow, m_display, m_rootWindow))
    {
        char* windowName = getWindowName(m_display,m_UOWindow);
        const std::string strWindowName(windowName);
        free(windowName);

        const UOClientType clitype = detectClientTypeFromTitle(
            strWindowName,
            getWindowTitleThirdpartyFragment() );
        if (clitype == m_clientType)
            return true;
        if (findUOWindow())
            return true;
    }

    m_error = KSError::NoWindow;
    return false;
}

bool KeystrokeSender::attach()
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

void KeystrokeSender::detach()
{
    if (m_display != NULL)
    {
        XCloseDisplay(m_display);
        m_display = NULL;
    }
}

bool KeystrokeSender::_sendChar(unsigned int ch)
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

bool KeystrokeSender::sendChar(unsigned int ch, bool setFocusToWindow)
{
    if (!attach())
        return false;

    if (setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendChar(ch);

    detach();
    return ret;
}

bool KeystrokeSender::_sendEnter()
{
    return sendChar((unsigned int)XK_Return);
}

bool KeystrokeSender::sendEnter(bool setFocusToWindow)
{
    if (!attach())
        return false;

    if (setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendEnter();

    detach();
    return ret;
}

bool KeystrokeSender::_sendString(const std::string& str, bool enterTerminated)
{
    const unsigned len = std::min(unsigned(str.length()), 255u);

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


bool KeystrokeSender::sendString(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    if ( str.length() < 1 )
    {
        m_error = KSError::StringShort;
        return false;
    }

    if (!attach())
        return false;

    if (setFocusToWindow)
        setForegroundWindow(m_display, m_UOWindow);

    bool ret = _sendString(str, enterTerminated);

    detach();
    return ret;
}


/*
 * Function which does all the stuff (using the X functions above). Keeping it for reference.
*/

/*
bool KeystrokeSender::sendChar(const char ch)
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
}



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
