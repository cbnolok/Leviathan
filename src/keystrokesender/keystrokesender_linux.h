#ifndef KEYSTROKESENDER_LINUX_H
#define KEYSTROKESENDER_LINUX_H

#ifndef _WIN32

#include "keystrokesender_common.h"


// forward declare Xlib types: we don't want to include Xlib.h here because it would pullute every
//  file including this header with tons of Xlib preprocessor macros. This would cause compilation errors
//  (conflict with Qt enum member names).
struct _XDisplay;
typedef _XDisplay 		Display;
typedef unsigned long	Window;

namespace keystrokesender
{


class KeystrokeSender_Linux
{
public:
    bool canSend();
    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char * const str, bool enterTerminated = true);

protected:
    KSError m_error = KSERR_OK;
    UOClientType m_clientType = CLIENT_UNK;

private:
    Display*   	m_display       = 0;	// NULL
    Window      m_rootWindow    = 0;
    Window      m_UOWindow      = 0;

    bool findUOWindow();
    bool _sendChar(const char ch);
};


}

#endif // !_WIN32

#endif // KEYSTROKESENDER_LINUX_H
