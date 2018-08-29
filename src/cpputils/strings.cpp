#include "strings.h"
#include <cstring>      // for strlen


void strToUpper(std::string &string)
{
    for (size_t i = 0; i < string.length(); ++i)
        string[i] = toupper(string[i]);
}

void strToUpper(char *string)
{
    for (size_t i = 0; i < strlen(string); ++i)
        string[i] = toupper(string[i]);
}


void strToLower(std::string &string)
{
    for (size_t i = 0; i < string.length(); ++i)
        string[i] = tolower(string[i]);
}

void strToLower(char *string)
{
    for (size_t i = 0; i < strlen(string); ++i)
        string[i] = tolower(string[i]);
}

void strTrim(std::string &string)
{
    static const char delims[] = " \t\n";

    size_t start = string.find_first_not_of(delims);        // Finds the first character equal to none of characters in delims
    if (start == std::string::npos)
        start = 0;

    size_t nchars = string.find_last_not_of(delims);        // Finds the last character equal to none of characters in delims
    if (nchars != std::string::npos)
        nchars = nchars - start + 1;

    string = string.substr(start, nchars);
}


bool isStringNumericHex(std::string &s)
{
    return !s.empty() && (s.find_first_not_of(" \t0123456789ABCDEFabcdef") == std::string::npos);
    /*
    char* p;
    strtol(s.c_str(), &p, 0);
    return *p == 0;
    */
}

bool isStringNumericDec(std::string &s)
{
    return !s.empty() && (s.find_first_not_of(" \t0123456789") == std::string::npos);
    /*
    char* p;
    strtol(s.c_str(), &p, 0);
    return *p == 0;
    */
}


