#ifndef _UOPERROR_H_
#define _UOPERROR_H_

#include <string>
#include <deque>

#define EXPORT
/*
#if defined(_MSC_VER)
    //  Microsoft
    #define EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
#else
    #define EXPORT
#endif
*/

namespace uoppackage
{


class EXPORT UOPError
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
    std::deque<std::string> getErrorQueue() const;
    bool errorOccurred() const;
    void clearErrorQueue();     // users have to manually clear the error queue between operations
};


extern UOPError errorHandler;

void addError(std::string errorString);


}

#endif
