#ifndef STRINGS_H
#define STRINGS_H

#include <string>


void strToUpper(std::string &string);   // Transforms the pased string to uppercase.
void strToUpper(char *string);
void strToLower(std::string &string);   // Transforms the pased string to lowercase.
void strToLower(char *string);
void strTrim(std::string &string);      // Remove leading and trailing spaces and newlines

bool isStringNumericHex(const std::string &s);
bool isStringNumericDec(const std::string &s);

std::wstring stringToWideString(const std::string& str);
std::string  wideStringToString(const std::wstring& wstr);


// Function by Marius: https://stackoverflow.com/questions/236129/split-a-string-in-c
template < class ContainerT >
void strSplit(const std::string& str, ContainerT& tokensContainer,
              const std::string& delimiters = " ", bool trimEmpty = true)
{
    std::string::size_type pos, lastPos = 0, length = str.length();

    using value_type = typename ContainerT::value_type;
    using size_type  = typename ContainerT::size_type;

    while(lastPos < length + 1)
    {
        pos = str.find_first_of(delimiters, lastPos);
        if(pos == std::string::npos)
            pos = length;

        if(pos != lastPos || !trimEmpty)
            tokensContainer.push_back(value_type(str.data()+lastPos, (size_type)pos-lastPos));

        lastPos = pos + 1;
    }
}



#endif // STRINGS_H
