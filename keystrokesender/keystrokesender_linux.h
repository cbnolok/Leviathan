#ifndef KEYSTROKESENDER_LINUX_H
#define KEYSTROKESENDER_LINUX_H

#ifndef _WIN32

#include "keystrokesender_common.h"
#include <X11/Xlib.h>


namespace keystrokesender
{


class KeystrokeSender_Linux
{
public:   
    bool sendChar(const char ch);
    bool sendEnter();
    bool sendString(const char * const str, bool enterTerminated = true);

protected:
    KSError m_error = KSERR_OK;
    UOClientType m_clientType = CLIENT_UNK;

private:
    Display*    m_display       = NULL;
    Window      m_rootWindow    = None;
    Window      m_UOWindow      = None;

    bool findUOWindow();
    bool canSend();
    bool _sendChar(const char ch);
};


}

#endif // !_WIN32

#endif // KEYSTROKESENDER_LINUX_H
