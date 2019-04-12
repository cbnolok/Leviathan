#if defined(__unix__) && defined(__APPLE__)

#include "keystrokesender_mac.h"

#define PLACEHOLDER void(0)
namespace ks
{

KeystrokeSender_Mac::KeystrokeSender_Mac(bool setFocusToWindow) :
    m_setFocusToWindow(setFocusToWindow)
{
}

/*
bool KeystrokeSender_Mac::_sendChar(const char ch)
{
    PLACEHOLDER;
}
*/

bool KeystrokeSender_Mac::sendChar(const char ch)
{
    PLACEHOLDER;
}

/*
bool KeystrokeSender_Mac::_sendEnter()
{
    PLACEHOLDER;
}
*/

bool KeystrokeSender_Mac::sendEnter()
{
    PLACEHOLDER;
}

/*
bool KeystrokeSender_Mac::_sendString(const std::string& str, bool enterTerminated)
{
    PLACEHOLDER;
}
*/

bool KeystrokeSender_Mac::sendString(const std::string& str, bool enterTerminated)
{
    PLACEHOLDER;
}

bool KeystrokeSender_Mac::sendStrings(const std::vector<std::string>& strings, bool enterTerminated)
{
    PLACEHOLDER;
}


/* static functions */

KSError KeystrokeSender_Mac::sendCharFast(const char ch, bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendEnterFast(bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendStringFast(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, bool setFocusToWindow)
{
    PLACEHOLDER;
}


/* Static async functions */

KSError KeystrokeSender_Mac::sendCharFastAsync(const char ch, bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendEnterFastAsync(bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendStringFastAsync(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    PLACEHOLDER;
}

KSError KeystrokeSender_Mac::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, bool setFocusToWindow)
{
    PLACEHOLDER;
}


}

#endif


// Snippets taken here and there from the web
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
