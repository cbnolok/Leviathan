#include "UOPError.h"
#include "zlib.h"


namespace uopp
{

UOPError errorHandler;


const std::deque<std::string> UOPError::getErrorQueue() const {
    return m_errorQueue;
}
bool UOPError::errorOccurred() const {
    return !m_errorQueue.empty();
}
void UOPError::clearErrorQueue() {
    m_errorQueue.clear();
}


}
