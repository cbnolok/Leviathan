#ifndef CPPUTILS_H
#define CPPUTILS_H

#include <string>
#include <map>

//#define COUNTOF(array) sizeof(array)/sizeof(array[0])
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

void strToUpper(std::string &string);   // Transforms the pased string to uppercase.
void strToUpper(char *string);
void strToLower(std::string &string);   // Transforms the pased string to lowercase.
void strToLower(char *string);
void strTrim(std::string &string);      // Remove leading and trailing spaces and newlines

bool isStringNumericHex(std::string &s);
bool isStringNumericDec(std::string &s);


template <typename key_t, typename value_t>
typename std::map<key_t,value_t>::iterator mapSearchByKey(std::map<key_t,value_t> &_map, const value_t _val)
// returns the iterator to the pair
{
    using it_t = typename std::map<key_t,value_t>::iterator;
    it_t end = _map.end();
    for (it_t it = _map.begin(); it != end; ++it)
    {
        if ( it->second == _val )
            return it;
    }
    return end;
}

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



#endif // COMMON_H
