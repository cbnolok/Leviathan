#include "exceptions.h"

namespace uocf
{

const char* UOCFException::what() const noexcept {
    return "UOCF generic exception";
}

InvalidStreamException::InvalidStreamException(const std::string& className, const std::string& message) {
    m_msg = '['+className+']' + " Invalid stream: " + message;
}
const char* InvalidStreamException::what() const noexcept {
    return m_msg.c_str();
}

NoCachePtrException::NoCachePtrException(const std::string& className) {
     m_msg = '['+className+']' + " Invalid cache pointer(s)";
}
const char* NoCachePtrException::what() const noexcept {
     return m_msg.c_str();
}

MalformedFileException::MalformedFileException(const std::string& className, const std::string& fileName) {
    m_msg = '['+className+']' + " Malformed file: " + fileName;
}
 const char* MalformedFileException::what() const noexcept {
    return m_msg.c_str();
}

UnsupportedActionException::UnsupportedActionException(const std::string& className, const std::string& fileName) {
    m_msg = '['+className+']' + " Malformed file: " + fileName;
}
 const char* UnsupportedActionException::what() const noexcept {
    return m_msg.c_str();
}

}
