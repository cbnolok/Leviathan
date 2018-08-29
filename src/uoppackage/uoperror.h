#ifndef UOPERROR_H
#define UOPERROR_H

#include <string>
#include <deque>


namespace uopp
{


class UOPError
{
    friend class UOPFile;
    friend class UOPHeader;
    friend class UOPBlock;
    friend class UOPPackage;

    using errorQueue_t = std::deque<std::string>;

public:
    //UOPError():
    //UOPError(const UOPError&);
    //void operator=(const UOPError&);

private:
    errorQueue_t m_errorQueue;
    static void append(const std::string& str, UOPError* errorQueue);

public:
    const errorQueue_t &getErrorQueue() const;
    bool errorOccurred() const;
    void clear();     // users have to manually clear the error queue between operations
    std::string operator[](int index) const;
    std::string buildErrorsString(bool emptyQueue = true, bool newLineFirst = false);
};


}

#endif // UOPERROR_H
