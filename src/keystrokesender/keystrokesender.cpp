#include "keystrokesender.h"


namespace keystrokesender
{

void KeystrokeSender::setSetFocusToWindow(bool value) {
    m_setFocusToWindow = value;
}

KSError KeystrokeSender::getError() const {
    return m_error;
}

std::string KeystrokeSender::getErrorString() const {
    return KSErrorString[m_error];
}

UOClientType KeystrokeSender::getClientType() const {
    return m_clientType;
}

}
