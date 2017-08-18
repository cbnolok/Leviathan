#include "keystrokesender.h"


namespace keystrokesender
{

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
