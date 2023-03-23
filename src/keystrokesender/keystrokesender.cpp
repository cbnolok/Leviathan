#include "keystrokesender.h"


namespace ks
{


void KeystrokeSender::setSetFocusToWindow(bool value) {
    m_setFocusToWindow = value;
}


char const * getErrorStringStatic(KSError err) {
    return KSErrorString[int(err)];
}

KSError KeystrokeSender::getError() const {
    return m_error;
}

std::string KeystrokeSender::getErrorString() const {
    return KSErrorString[int(m_error)];
}

UOClientType KeystrokeSender::getClientType() const {
    return m_clientType;
}


}
