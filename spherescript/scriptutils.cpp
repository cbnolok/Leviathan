#include "scriptutils.h"
#include "common.h"
#include <cstring>      // for strcmp
#include <stdexcept>    // for std::invalid_argument


const char * ScriptUtils::resourceBlocks[ScriptUtils::SCRIPTOBJ_RES_QTY] =
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
};


//Default Script Objects we have to deal with
const char * ScriptUtils::objectTags[ScriptUtils::TAG_QTY] =
{
    // these strings have type const char *, so the variable linked to these "table" entries
    //  is a const char * const * (pointer to const pointer to const char)
    "CATEGORY",
    "COLOR",
    "DEFNAME",
    "DESCRIPTION",
    "DUPEITEM",
    "GROUP",
    "ID",
    "NAME",
    "SUBSECTION",
    "P",
    "POINT",
};


int ScriptUtils::findTable(const char * stringToFind, const char * const * table, int count)
{                                                   // non const pointer to a const pointer to const char
    // A non-sorted table.
    // RETURN: -1 = not found

    char * stringToFindUpper = (char*)malloc( (strlen(stringToFind)+1) * sizeof(char) );
    strcpy(stringToFindUpper, stringToFind);
    strToUpper(stringToFindUpper);

    for (int i = 0; i < count; i++)
    {
        if (!strcmp(table[i], stringToFindUpper))
        {
            free(stringToFindUpper);
            return i;
        }
    }
    free(stringToFindUpper);
    return -1;
}

int ScriptUtils::findTableSorted(const char * stringToFind, const char * const * table, int count)
{
    // Do a binary search (un-cased) on a sorted table.
    // RETURN: -1 = not found
    int iHigh = count - 1;
    if (iHigh < 0)
        return -1;
    int iLow = 0;

    char * stringToFindUpper = (char*)malloc( (strlen(stringToFind)+1) * sizeof(char) );
    strcpy(stringToFindUpper, stringToFind);
    strToUpper(stringToFindUpper);

    while (iLow <= iHigh)
    {
        int i = (iHigh + iLow) / 2;
        int compare = strcmp(table[i], (const char*)stringToFindUpper);
        if (compare == 0)
        {
            free(stringToFindUpper);
            return i;
        }
        if (compare > 0)
            iHigh = i - 1;
        else
            iLow = i + 1;
    }

    free(stringToFindUpper);
    return -1;
}

int ScriptUtils::strToSphereInt(std::string str)
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
        ret = std::stoi(str, 0 , base);
    }
    catch (std::invalid_argument e)
    {
        ret = -1;        // If no valid conversion can be done, return -1.
    }

    return ret;
}

int ScriptUtils::strToSphereInt(const char *str)
{
    return strToSphereInt(std::string(str));
}



