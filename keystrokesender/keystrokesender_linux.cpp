#ifndef _WIN32

#include "keystrokesender_linux.h"
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

#include <X11/keysym.h>


// Based on:
// Send a fake keystroke event to an X window.
// by Adam Pierce - http://www.doctort.org/adam/
// This is public domain software. It is free to use by anyone for any purpose.

// And on:
// https://stackoverflow.com/questions/30156202/xlib-c-get-window-handle-sendevent


// !!! To compile this file, the following linker instructions are needed: -L/usr/X11R6/lib -lX11 !!!


namespace keystrokesender
{


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

    for (unsigned long i = 0; i < windowListSize; i++)
    {
        if (list[i] == windowToFind)
            return true;
    }
    return false;
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
    for (unsigned long i = 0; i < windowsCount; i++)
    {
        char* windowName = getWindowName(m_display,list[i]);
        std::string strWindowName(windowName);
        free(windowName);

        if ( strWindowName.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos )
            m_UOWindow = list[i];
        else if ( strWindowName.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos )
            m_UOWindow = list[i];

        if ( m_UOWindow != None )
        {
            m_error = KSERR_OK;
            return true;
        }
    }

    m_error = KSERR_NOWINDOW;
    return false;
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
    // Obtain the X11 display.
    m_display = XOpenDisplay(NULL);
    if (m_display == NULL)
        return false;

    // Get the root window for the current display.
    m_rootWindow = XDefaultRootWindow(m_display);

    // Find the window where to send the keystroke.
    if ( !findUOWindow() )
        return false;

    bool ret = _sendChar(ch);

    // Done.
    XCloseDisplay(m_display);
    m_display = NULL;

    return ret;
}

bool KeystrokeSender_Linux::sendEnter()
{
    return sendChar((char)XK_Return);
}

bool KeystrokeSender_Linux::sendString(const char * const str, bool enterTerminated)
{
    if ( (strlen(str) < 1) )
    {
        m_error = KSERR_STRINGSHORT;
        return false;
    }

    int len = (strlen(str) > 255) ? 255 : strlen(str);

    // Obtain the X11 display.
    m_display = XOpenDisplay(NULL);
    if (m_display == NULL)
        return false;

    // Get the root window for the current display.
    m_rootWindow = XDefaultRootWindow(m_display);

    // Find the window where to send the keystroke.
    if ( !findUOWindow() )
        return false;

    bool ret = true;
    for (int i = 0; i < len; i++)
    {
        if ( !_sendChar(str[i]) )
        {
            ret = false;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    if (enterTerminated)
    {
        if ( !_sendChar((char)XK_Return) )
            ret = false;
    }

    // Done.
    XCloseDisplay(m_display);
    m_display = NULL;

    return ret;
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
    for (unsigned long i = 0; i < windowsCount; i++)
    {
        windowName = getWindowName(display,list[i]);
        std::string strWindowName(windowName);
        free(windowName);
        //std::cout << i << ": " << strWindowName << std::endl;
        if ( strWindowName.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos )
            UOWindow = &list[i];
        else if ( strWindowName.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos )
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

#endif // !_WIN32


/* Mac OSX */   // Snippets taken here and there from the web
/*

#include <ApplicationServices/ApplicationServices.h>

void Press(int key);
void Release(int key);
void Click(int key);

int main() {
    Press(56);
    Click(6);
    Release(56);
}

void Press(int key) {
    // Create an HID hardware event source
    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    // Create a new keyboard key press event
    CGEventRef evt = CGEventCreateKeyboardEvent(src, (CGKeyCode) key, true);

    // Post keyboard event and release
    CGEventPost (kCGHIDEventTap, evt);
    CFRelease (evt); CFRelease (src);

    usleep(60);
}

void Release(int key) {
    // Create an HID hardware event source
    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    // Create a new keyboard key release event
    CGEventRef evt = CGEventCreateKeyboardEvent(src, (CGKeyCode) key, false);

    // Post keyboard event and release
    CGEventPost (kCGHIDEventTap, evt);
    CFRelease (evt); CFRelease (src);

    usleep(60);
}

void Click(int key) {
    Press(key);
    Release(key);
}


------------------------------------

// From https://stackoverflow.com/questions/2379867/simulating-key-press-events-in-mac-os-x, by Dave DeLong
Here's code to simulate a Cmd-S action:

CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
CGEventRef saveCommandDown = CGEventCreateKeyboardEvent(source, (CGKeyCode)1, YES);
CGEventSetFlags(saveCommandDown, kCGEventFlagMaskCommand);
CGEventRef saveCommandUp = CGEventCreateKeyboardEvent(source, (CGKeyCode)1, NO);

CGEventPost(kCGAnnotatedSessionEventTap, saveCommandDown);
CGEventPost(kCGAnnotatedSessionEventTap, saveCommandUp);

CFRelease(saveCommandUp);
CFRelease(saveCommandDown);
CFRelease(source);

A CGKeyCode is nothing more than an unsigned integer:

typedef uint16_t CGKeyCode;  //From CGRemoteOperation.h

Your real issue will be turning a character (probably an NSString) into a keycode.
*/
