#include "scriptutils.h"
#include "common.h"
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
        ret = stoi(str, 0 , base);
    }
    catch (std::invalid_argument e)
    {
        ret = -1;        // If no valid conversion can be done, return -1.
    }

    return ret;
}

int ScriptUtils::strToSphereInt(const char *str)
{
    std::string stdstr(str);
    return strToSphereInt(stdstr);
}

int ScriptUtils::strToSphereInt16(std::string str)
{
    int temp = ScriptUtils::strToSphereInt(str);
    return (temp > (int)UINT16_MAX) ? 0 : temp;
}

int ScriptUtils::strToSphereInt16(const char *str)
{
    std::string stdstr(str);
    return strToSphereInt16(stdstr);
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(int num)
{
    std::stringstream stream;
    stream << "0" << std::noshowbase << std::hex << num;
    return std::string(stream.str());
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(std::string &str)  // return a numerical string formatted as a sphere hex number ("0123")
{
    int num = strToSphereInt(str);
    if (num == -1)
        return std::string("-1");
    return numericalStrFormattedAsSphereInt(num);
}

std::string ScriptUtils::numericalStrFormattedAsSphereInt(const char *str)
{
    std::string stdstr(str);
    return numericalStrFormattedAsSphereInt(stdstr);
}


//-------------

int ScriptUtils::findTableSorted(std::string stringToFind, std::vector<const char*> &table, int tableSize)
{
    // Do a binary search (un-cased) on a sorted table.
    // RETURN: -1 = not found

    // Not using a std::vector<std::string> table because string.compare method is slower than strcmp

    //int iHigh = table.size() - 1; // slower than manually passing the tableSize? (implicit cast form size_t to int)
    int iHigh = tableSize - 1;
    if (iHigh < 0)
        return -1;
    int iLow = 0;

    // std::string automatically stores the length, so there's no need to use C string + strlen when uppercasing the string
    strToUpper(stringToFind);
    const char* stringToFindC = stringToFind.c_str();

    while (iLow <= iHigh)
    {
        int i = (iHigh + iLow) >> 1; // / 2;

        int compare = strcmp(stringToFindC, table[i]);

        if (compare == 0)
            return i;
        else if (compare < 0)
            iHigh = i - 1;
        else
            iLow = i + 1;
    }

    return -1;
}


std::vector<const char*> ScriptUtils::resourceBlocks =
{
    "AAAUNUSED",        // unused / unknown.
    "ACCOUNT",          // Define an account instance.
    "ADVANCE",          // Define the advance rates for stats.
    "AREA",             // Complex region. (w/ extra tags)
    "AREADEF",          // Complex region. (w/extra tags)       // compatibility    -> SCRIPTOBJ_RES_AREA
    "BLOCKIP",          // (SL) A list of IP's to block.
    "BOOK",             // A book or a page from a book.
    "CHARDEF",          // Define a char type.
    "COMMENT",          // A commented out block type.
    "DEFMESSAGE",       // New 56a message block                // compatibility    -> SCRIPTOBJ_RES_DEFNAME
    "DEFNAME",          // (SL) Just add a bunch of new defs and equivs str/values.
    "DIALOG",			// A scriptable gump dialog", text or handler block.
    "EVENTS",			// (SL) Preload these Event files.
    "FAME",
    "FUNCTION",         // Define a new command verb script that applies to a char.
    "GLOBALS",                                                  // compatibility    -> SCRIPTOBJ_RES_WORLDVARS
    "GMPAGE",			// A GM page. (SAVED in World)
    "ITEMDEF",          // Define an item type
    "KARMA",
    "KRDIALOGLIST",     // mapping of dialog<->kr ids
    "LISTS",                                                    // compatibility    -> SCRIPTOBJ_RES_WORLDLISTS
    "MENU",             // General scriptable menus.
    "MOONGATES",		// (SL) Define where the moongates are.
    "MULTIDEF",                                                 // compatibility    -> SCRIPTOBJ_RES_ITEMDEF
    "NAMES",            // A block of possible names for a NPC type. (read as needed)
    "NEWBIE",			// Triggers to execute on Player creation (based on skills selected)
    "NOTOTITLES",		// (SI) Define the noto titles used.
    "OBSCENE",          // (SL) A list of obscene words.
    "PLEVEL",			// Define the list of commands that a PLEVEL can access. (or not access)
    "REGIONRESOURCE",	// Define Ore types.
    "REGIONTYPE",       // Triggers etc. that can be assinged to a "AREA
    "RESOURCELIST",
    "RESOURCES",		// (SL) list of all the resource files we should index !
    "ROOM",             // Non-complex region. (no extra tags)
    "ROOMDEF",			// Non-complex region. (no extra tags)  // compatibility    -> SCRIPTOBJ_RES_ROOM
    "RUNES",			// (SI) Define list of the magic runes.
    "SCROLL",			// SCROLL_GUEST=message scroll sent to player at guest login. SCROLL_MOTD", SCROLL_NEWBIE
    "SECTOR",			// Make changes to a sector. (SAVED in World)
    "SERVERS",          // List a number of servers in 3 line format.
    "SKILL",			// Define attributes for a skill (how fast it raises etc)
    "SKILLCLASS",		// Define class specifics for a char with this skill class.
    "SKILLMENU",		// A menu that is attached to a skill. special arguments over other menus.
    "SPAWN",			// Define a list of NPC's and how often they may spawn.
    "SPEECH",			// (SL) Preload these speech files.
    "SPELL",			// Define a magic spell. (0-64 are reserved)
    "SPHERE",			// Main Server INI block
    "SPHERECRYPT",      // Encryption keys
    "STARTS",			// (SI) List of starting locations for newbies.
    "STAT",             // Stats elements like KARMA,STR,DEX,FOOD,FAME,CRIMINAL etc. Used for resource and desire scripts.
    "TELEPORTERS",      // (SL) Where are the teleporteres in the world ?
    "TEMPLATE",         // Define a list of items. (for filling loot etc)
    "TIMERF",
    "TIP",              // Tips that can come up at startup.
    "TYPEDEF",			// Define a trigger block for a "WORLDITEM m_type.
    "TYPEDEFS",
    "WC",				// =WORLDCHAR
    "WEBPAGE",          // Define a web page template.
    "WI",				// =WORLDITEM
    "WORLDCHAR",		// Define instance of char in the world. (SAVED in World)
    "WORLDITEM",		// Define instance of item in the world. (SAVED in World)
    "WORLDLISTS",		// Define instance of list in the world. (SAVED in World)
    "WORLDSCRIPT",		// Define instance of resource in the world. (SAVED in World)
    "WORLDVARS",		// block of global variables
    "WS",				// =WORLDSCRIPT
    ""
};


std::vector<const char*> ScriptUtils::objectTags
{
    // these strings have type const char *, so the variable linked to these "table" entries
    //  is a const char * const * (pointer to const pointer to const char)
    "CATEGORY",
    "COLOR",
    "DEFNAME",
    "DESCRIPTION",
    "DUPEITEM",
    "DUPELIST",
    "GROUP",
    "ID",
    "NAME",
    "SUBSECTION",
    "P",
    "POINT",
    ""
};


