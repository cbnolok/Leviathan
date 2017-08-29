#ifndef _UOPERROR_H_
#define _UOPERROR_H_

#include <string>
#include <deque>


namespace uoppackage
{


class UOPError
{
    friend class UOPFile;
    friend class UOPHeader;
    friend class UOPBlock;
    friend class UOPPackage;

public:
    UOPError() { }
    //UOPError(const UOPError&);
    //void operator=(const UOPError&);

private:
    std::deque<std::string> m_errorQueue;

public:
    const std::deque<std::string> getErrorQueue() const;
    bool errorOccurred() const;
    void clearErrorQueue();     // users have to manually clear the error queue between operations
};


extern UOPError errorHandler;


}

#endif
