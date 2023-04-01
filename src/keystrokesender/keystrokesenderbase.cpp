#include "keystrokesenderbase.h"
#include "keystrokesender.h"
#include <thread>


#define STATIC_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

namespace ks
{

/* namespace-level functions */

const char * const KSErrorString[]
{   // The string order must match the UOClientComError enum order.
    "No error.",
    "UO Client window was not found.",
    "String too short."
};

static const std::string UODefaultClientWindowTitles[]
{   // The string order must match the UOClientType enum order.
    "Ultima Online",      // for Classic Client
    "UOSA -",             // for Enhanced Client
    ""
};

char const * getErrorStringStatic(KSError err) {

    if (int(err) < 0 || (int(err) >= int(STATIC_ARRAY_COUNT(KSErrorString))) )
        return "";
    return KSErrorString[int(err)];
}

UOClientType detectClientTypeFromTitle(std::string const& windowName, std::string const& windowtitleThirdpartyFragment)
{
    if (windowtitleThirdpartyFragment.empty())
    {
        if (std::string::npos != windowName.find(UODefaultClientWindowTitles[(int)UOClientType::Classic]))
            return UOClientType::Classic;
        if (std::string::npos != windowName.find(UODefaultClientWindowTitles[(int)UOClientType::Enhanced]))
            return UOClientType::Enhanced;
    }
    else if (std::string::npos != windowName.find(windowtitleThirdpartyFragment))
    {
        return UOClientType::ThirdParty;
    }

    return UOClientType::Unknown;
}


/* base class methods */

KeystrokeSenderBase::KeystrokeSenderBase(std::string windowTitleFragment) :
    m_error(KSError::Ok), m_clientType(UOClientType::Unknown),
    m_windowTitleThirdpartyFragment(windowTitleFragment)
{

}

KSError KeystrokeSenderBase::getError() const {
    return m_error;
}

std::string KeystrokeSenderBase::getErrorString() const {
    return KSErrorString[int(m_error)];
}

UOClientType KeystrokeSenderBase::getClientType() const {
    return m_clientType;
}

std::string KeystrokeSenderBase::getWindowTitleThirdpartyFragment() const
{
    return m_windowTitleThirdpartyFragment;
}

bool KeystrokeSenderBase::sendStrings(const std::vector<std::string>& strings, bool enterTerminated, bool setFocusToWindow)
{
    for (const std::string& str : strings)
    {
        if (!sendString(str, enterTerminated, setFocusToWindow))
            return false;
    }

    return true;
}


/* static functions */

KSError KeystrokeSenderBase::sendCharFast(const char ch, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    ks.sendChar(ch, setFocusToWindow);
    return ks.getError();
}

KSError KeystrokeSenderBase::sendEnterFast(std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    ks.sendEnter(setFocusToWindow);
    return ks.getError();
}

KSError KeystrokeSenderBase::sendStringFast(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    ks.sendString(str, enterTerminated, setFocusToWindow);
    return ks.getError();
}

KSError KeystrokeSenderBase::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    for (const std::string& str : strings)
    {
        ks.sendString(str, enterTerminated, setFocusToWindow);
        if (ks.m_error != KSError::Ok)
            return ks.getError();
    }
    return ks.getError();
}


/* Static async functions */

KSError KeystrokeSenderBase::sendCharFastAsync(const char ch, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    if (!ks.canSend())
        return ks.getError();

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [ch, setFocusToWindow, ks = KeystrokeSender(windowTitleFragment)] () mutable -> void
        {
            ks.sendChar(ch, setFocusToWindow);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSenderBase::sendEnterFastAsync(std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    if (!ks.canSend())
        return ks.getError();

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [setFocusToWindow, ks = KeystrokeSender(windowTitleFragment)] () mutable -> void
        {
            ks.sendEnter(setFocusToWindow);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSenderBase::sendStringFastAsync(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    if (!ks.canSend())
        return ks.getError();

    if ( str.length() < 1 )
        return KSError::StringShort;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [str, enterTerminated, setFocusToWindow, ks = KeystrokeSender(windowTitleFragment)] () mutable -> void
        {
            ks.sendString(str, enterTerminated, setFocusToWindow);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSenderBase::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow)
{
    KeystrokeSender ks(windowTitleFragment);
    if (!ks.canSend())
        return ks.getError();

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [strings, enterTerminated, setFocusToWindow, ks = KeystrokeSender(windowTitleFragment)] () mutable -> void
        {
            ks.sendStrings(strings, enterTerminated, setFocusToWindow);
        });
    sender.detach();

    return KSError::Ok;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}


}
