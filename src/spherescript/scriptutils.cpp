#include "scriptutils.h"

#include <cstring>      // for strcmp
#include <sstream>      // for std::stringstream
#include <iomanip>      // for std::hex
#include <stdexcept>    // for std::invalid_argument


int ScriptUtils::strToSphereInt(std::string str)   // it's important to work on a copy, since we insert a character in this function
{
    // Sphere deals as hexadecimal numbers numerical strings starting both with 0 and 0x, otherwise they are decimals.
    // This function is needed because stoi deals with numbers starting with 0 as octal, and only 0x as hex.

    if (str.empty())
        return -1;

    int base = 10;
    if (str[0]=='0')    // It's an hexadecimal number, or it's simply zero.
    {
        if (str.length() > 1)
        {
            base = 16;

            // If the hex number isn't in the format "0x..." but is in the format "0...", convert it to the first one, so that
            //  stoi can recognize it properly.
            if ( (str[1] != 'x') && (str[1]!='X') )
                str.insert(1, 1, 'x');
        }
    }
    int ret;
    try
    {
        ret = std::stoi(str, nullptr, base);
    }
    catch (const std::invalid_argument&)
    {
        ret = -1;        // If no valid conversion can be done, return -1.
    }

    return ret;
}

int ScriptUtils::strToSphereInt(const char *str)
{
    return strToSphereInt(std::string(str));
}

int ScriptUtils::strToSphereInt16(std::string str)
{
    int temp = ScriptUtils::strToSphereInt(str);
    return (temp > (int)UINT16_MAX) ? 0 : temp;
}

int ScriptUtils::strToSphereInt16(const char *str)
{
    return strToSphereInt16(std::string(str));
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(int num)
{
    std::stringstream stream;
    stream << "0" << std::noshowbase << std::hex << num;
    return stream.str();
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(const std::string &str)  // return a numerical string formatted as a sphere hex number ("0123")
{
    int num = strToSphereInt(str);
    if (num == -1)
        return std::string("-1");
    return numericalStrFormattedAsSphereInt(num);
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(const char *str)
{
    return numericalStrFormattedAsSphereInt(std::string(str));
}


//-------------

#ifdef _WIN32
#define stricmp _stricmp
#else
#define stricmp strcasecmp
#endif

int ScriptUtils::findTableSorted(std::string_view stringToFind, std::span<const std::string_view> table, const int tableSize) noexcept
{
    // Do a binary search (un-cased) on a sorted table.
    // RETURN: -1 = not found

    //int iHigh = table.size() - 1;
    int iHigh = tableSize - 1;
    if (iHigh < 0)
        return -1;
    int iLow = 0;

    while (iLow <= iHigh)
    {
        const int i = (iHigh + iLow) >> 1; // / 2;
        const int compare = stricmp(stringToFind.data(), table[i].data());

        if (compare == 0)
            return i;
        else if (compare < 0)
            iHigh = i - 1;
        else
            iLow = i + 1;
    }

    return -1;
}


