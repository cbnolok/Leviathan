#include "strings.h"
#include <cstring>      // for strlen
#include <algorithm>    // std::any_of
#include <functional>   // std::negate


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


bool isStringNumericHex(const std::string &s)
{
    return !s.empty() && (s.find_first_not_of(" \t0123456789ABCDEFabcdef") == std::string::npos);
    /*
    char* p;
    strtol(s.c_str(), &p, 0);
    return *p == 0;
    */
}

bool isStringNumericDec(const std::string &s)
{
    return !s.empty() && (s.find_first_not_of(" \t0123456789") == std::string::npos);
    /*
    char* p;
    strtol(s.c_str(), &p, 0);
    return *p == 0;
    */
}

bool isStringASCII(const char* str)
{
    while (*str != '\0')
    {
        if (!isCharASCII(*str))
            return false;
        ++str;
    }
    return true;
}

bool isStringASCII(std::string const& str)
{
    return std::any_of(str.cbegin(), str.cend(), std::not_fn(isCharASCII));
}



// Converting WStrings to Strings, and vice-versa.
// https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
#include <locale>

// WINDOWS
#if (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define WINDOWS_PLATFORM 1

//EMSCRIPTEN
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <unistd.h>
#define EMSCRIPTEN_PLATFORM 1

// LINUX
#elif (__LINUX__ || __gnu_linux__ || __linux__ || __linux || linux)
#define LINUX_PLATFORM 1
#include <unistd.h>
#define CoTaskMemAlloc(p) malloc(p)
#define CoTaskMemFree(p) free(p)

//ANDROID
#elif (__ANDROID__ || ANDROID)
#define ANDROID_PLATFORM 1

//MACOS
#elif defined(__APPLE__)
#include <unistd.h>
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#define IOS_SIMULATOR_PLATFORM 1
#elif TARGET_OS_IPHONE
#define IOS_PLATFORM 1
#elif TARGET_OS_MAC
#define MACOS_PLATFORM 1
#endif

#endif

using namespace std::literals::string_literals;
std::string wideStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return {};
    }
    size_t pos;
    size_t begin = 0;
    std::string ret;

#if WINDOWS_PLATFORM
    int size;
    pos = wstr.find(static_cast<wchar_t>(0), begin);
    while (pos != std::wstring::npos && begin < wstr.length())
    {
        std::wstring segment(&wstr[begin], pos - begin);
        size = static_cast<int>(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], static_cast<int>(segment.size()),
                                                    nullptr, 0, nullptr, nullptr));
        std::string converted(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], static_cast<int>(segment.size()),
                            &converted[0], static_cast<int>(converted.size()), nullptr, nullptr);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
    }
    if (begin <= wstr.length())
    {
        std::wstring segment(&wstr[begin], wstr.length() - begin);
        size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], static_cast<int>(segment.size()),
                                   nullptr, 0, nullptr, nullptr);
        std::string converted(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &segment[0], static_cast<int>(segment.size()),
                            &converted[0], static_cast<int>(converted.size()), nullptr, nullptr);
        ret.append(converted);
    }
#elif LINUX_PLATFORM || MACOS_PLATFORM || EMSCRIPTEN_PLATFORM
    size_t size;

    while (pos != std::wstring::npos && begin < wstr.length())
    {
        std::wstring segment = std::wstring(&wstr[begin], pos - begin);
        pos = wstr.find(static_cast<wchar_t>(0), begin);
        size = wcstombs(nullptr, segment.c_str(), 0);
        std::string converted = std::string(size, 0);
        wcstombs(&converted[0], segment.c_str(), converted.size());
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = wstr.find(static_cast<wchar_t>(0), begin);
    }
    if (begin <= wstr.length())
    {
        std::wstring segment = std::wstring(&wstr[begin], wstr.length() - begin);
        size = wcstombs(nullptr, segment.c_str(), 0);
        std::string converted = std::string(size, 0);
        wcstombs(&converted[0], segment.c_str(), converted.size());
        ret.append(converted);
    }
#else
    static_assert(false, "Unknown Platform");
#endif
    return ret;
}

std::wstring stringToWideString(const std::string& str)
{
    if (str.empty())
    {
        return {};
    }

    size_t pos;
    size_t begin = 0;
    std::wstring ret;
#ifdef WINDOWS_PLATFORM
    int size = 0;
    pos = str.find(static_cast<char>(0), begin);
    while (pos != std::string::npos)
    {
        std::string segment(&str[begin], pos - begin);
        std::wstring converted(segment.size() + 1, 0);
        size = static_cast<int>(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &segment[0], static_cast<int>(segment.size()),
                                &converted[0], static_cast<int>(converted.length())));
        converted.resize(size);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = str.find(static_cast<char>(0), begin);
    }
    if (begin < str.length())
    {
        std::string segment(&str[begin], str.length() - begin);
        std::wstring converted(segment.size() + 1, 0);
        size = static_cast<int>(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, segment.c_str(), static_cast<int>(segment.size()),
                                                    &converted[0], static_cast<int>(converted.length())));
        converted.resize(size);
        ret.append(converted);
    }

#elif LINUX_PLATFORM || MACOS_PLATFORM || EMSCRIPTEN_PLATFORM
    size_t size;
    pos = str.find(static_cast<char>(0), begin);
    while (pos != std::string::npos)
    {
        std::string segment = std::string(&str[begin], pos - begin);
        std::wstring converted = std::wstring(segment.size(), 0);
        size = mbstowcs(&converted[0], &segment[0], converted.size());
        converted.resize(size);
        ret.append(converted);
        ret.append({ 0 });
        begin = pos + 1;
        pos = str.find(static_cast<char>(0), begin);
    }
    if (begin < str.length())
    {
        std::string segment = std::string(&str[begin], str.length() - begin);
        std::wstring converted = std::wstring(segment.size(), 0);
        size = mbstowcs(&converted[0], &segment[0], converted.size());
        converted.resize(size);
        ret.append(converted);
    }
#else
    static_assert(false, "Unknown Platform");
#endif
    return ret;
};
