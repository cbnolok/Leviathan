#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <exception>


namespace uocf
{

struct UOCFException : public std::exception
{
    virtual const char* what() const noexcept;
};

class InvalidStreamException : public UOCFException
{
    std::string m_msg;
 public:
    InvalidStreamException(const std::string &className, const std::string& message);
    virtual const char* what() const noexcept;
};

class NoCachePtrException : public UOCFException
{
    std::string m_msg;
 public:
    NoCachePtrException(const std::string &className);
    virtual const char* what() const noexcept;
};

class MalformedFileException : public UOCFException
{
    std::string m_msg;
 public:
    MalformedFileException(const std::string &className, const std::string& fileName);
    virtual const char* what() const noexcept;
};

class UnsupportedActionException : public UOCFException
{
    std::string m_msg;
 public:
    UnsupportedActionException(const std::string &className, const std::string& fileName);
    virtual const char* what() const noexcept;
};

}

#endif // EXCEPTIONS_H
