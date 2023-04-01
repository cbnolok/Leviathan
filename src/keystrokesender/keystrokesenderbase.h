#ifndef KEYSTROKESENDERBASE_H
#define KEYSTROKESENDERBASE_H

#include <string>
#include <vector>

namespace ks
{

enum class UOClientType
{
    Unknown     = -1,
    Classic     = 0,
    Enhanced    = 1,
    ThirdParty  = 2
};

enum class KSError
{
    Ok          = 0,    // no error
    NoWindow,
    StringShort,
};

extern const char * const KSErrorString[];


// Namespace-level functions
const char * getErrorStringStatic(KSError err);
UOClientType detectClientTypeFromTitle(std::string const& windowName, std::string const& windowtitleThirdpartyFragment);


class KeystrokeSenderBase
{
public:
    KeystrokeSenderBase(std::string windowTitleFragment);
    ~KeystrokeSenderBase() = default;

    // Abstract methods
    //virtual void resetWindow() = 0;
    virtual bool canSend() = 0;

    virtual bool sendChar(unsigned int ch, bool setFocusToWindow = false) = 0;
    virtual bool sendEnter(bool setFocusToWindow = false) = 0;
    virtual bool sendString(const std::string& str, bool enterTerminated = true, bool setFocusToWindow = false) = 0;

    // Base methods
    KSError getError() const;
    std::string getErrorString() const;
    UOClientType getClientType() const;
    std::string getWindowTitleThirdpartyFragment() const;
    bool sendStrings(const std::vector<std::string> &strings, bool enterTerminated = true, bool setFocusToWindow = false);

public:
    // Static methods: do the work without creating an instance of the class and setting everything manually
    static KSError sendCharFast(const char ch, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendEnterFast(std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringFast(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);

    // Static async methods: spawn a thread to do this, so we don't have to pause the current thread
    static KSError sendCharFastAsync(const char ch, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendEnterFastAsync(std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringFastAsync(const std::string& str, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);
    static KSError sendStringsFastAsync(const std::vector<std::string>& strings, bool enterTerminated, std::string windowTitleFragment, bool setFocusToWindow = false);

protected:
    KSError m_error;
    UOClientType m_clientType;
    std::string m_windowTitleThirdpartyFragment;

};


}

#endif // KEYSTROKESENDERBASE_H
