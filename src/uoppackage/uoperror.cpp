#include "uoperror.h"
#include <sstream>


namespace uopp
{


void UOPError::append(const std::string& str, UOPError* errorQueue) // static
{
    if (errorQueue != nullptr)
        errorQueue->m_errorQueue.emplace_back(str);
}

const UOPError::errorQueue_t &UOPError::getErrorQueue() const {
    return m_errorQueue;
}
bool UOPError::errorOccurred() const {
    return !m_errorQueue.empty();
}
void UOPError::clear() {
    m_errorQueue.clear();
}

std::string UOPError::operator[](unsigned int index) const
{
    if (index < m_errorQueue.size())
        return m_errorQueue[index];
    return {};
}

std::string UOPError::buildErrorsString(bool emptyQueue, bool newLineFirst)
{
    std::stringstream ssErrors;
    if (newLineFirst)
        ssErrors << '\n';
    unsigned int i = 1;
    for (const auto& curString : m_errorQueue)
    {
        ssErrors << i << ") " << curString << '\n';
        ++i;
    }
    if (i == 1)
        return "None";
    if (emptyQueue)
        clear();
    return ssErrors.str();
}


} // end of uopp namespace
