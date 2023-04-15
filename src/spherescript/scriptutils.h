#ifndef SCRIPTUTILS_H
#define SCRIPTUTILS_H

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>


class ScriptUtils
{
public:
    static int strToSphereInt(std::string str);     // In sphere numbers can be decimal or hexadecimal, not octal.
    static int strToSphereInt(const char *str);

    static int strToSphereInt16(std::string str);   // Utility function to convert hue from string to number (which has actually max size 2 bytes [short], not 4 [int])
    static int strToSphereInt16(const char *str);

    static std::string numericalStrFormattedAsSphereInt(int num);  // Return a numerical string formatted as a sphere hex number ("0123")
    static std::string numericalStrFormattedAsSphereInt(const std::string& str);
    static std::string numericalStrFormattedAsSphereInt(const char *str);


    static int findTableSorted(std::string_view stringToFind, std::span<const std::string_view> table, const int tableSize) noexcept;


    // All the script resource blocks in SphereServer.

    enum SCRIPTOBJ_RES_TYPE
    {
        // NOTE: SPHERE.INI, SPHERETABLE.SCP are read at start.
        // All other files are indexed from the SCPFILES directories.
        // (SI) = Single instance types.
        // (SL) = single line multiple definitions.
        // Alphabetical order.
        SCRIPTOBJ_RES_UNKNOWN = 0,	// Not to be used.
        SCRIPTOBJ_RES_ACCOUNT,		// Define an account instance.
        SCRIPTOBJ_RES_ADVANCE,		// Define the advance rates for stats.
        SCRIPTOBJ_RES_AREA,			// Complex region. (w/extra tags)
        SCRIPTOBJ_RES_AREADEF,		// Complex region. (w/extra tags)   // compatibility    -> SCRIPTOBJ_RES_AREA
        SCRIPTOBJ_RES_BLOCKIP,		// (SL) A list of IP's to block.
        SCRIPTOBJ_RES_BOOK,			// A book or a page from a book.
        SCRIPTOBJ_RES_CHARDEF,		// Define a char type. (overlap with SCRIPTOBJ_RES_SPAWN)
        SCRIPTOBJ_RES_COMMENT,		// A commented out block type.
        SCRIPTOBJ_RES_DEFMESSAGE,		// New 56a message block            // compatibility    -> SCRIPTOBJ_RES_DEFNAME
        SCRIPTOBJ_RES_DEFNAME,		// (SL) Just add a bunch of new defs and equivs str/values.
        SCRIPTOBJ_RES_DIALOG,			// A scriptable gump dialog, text or handler block.
        SCRIPTOBJ_RES_EVENTS,			// An Event handler block with the trigger type in it. ON=@Death etc.
        SCRIPTOBJ_RES_FAME,
        SCRIPTOBJ_RES_FUNCTION,		// Define a new command verb script that applies to a char.
        SCRIPTOBJ_RES_GLOBALS,                                            // compatibility    -> SCRIPTOBJ_RES_WORLDVARS
        SCRIPTOBJ_RES_GMPAGE,			// A GM page. (SAVED in World)
        SCRIPTOBJ_RES_ITEMDEF,		// Define an item type. (overlap with SCRIPTOBJ_RES_TEMPLATE)
        SCRIPTOBJ_RES_KARMA,
        SCRIPTOBJ_RES_KRDIALOGLIST,	// Mapping of dialog<->kr ids
        SCRIPTOBJ_RES_LISTS,                                              // compatibility    -> SCRIPTOBJ_RES_WORLDLISTS
        SCRIPTOBJ_RES_MENU,			// General scriptable menus.
        SCRIPTOBJ_RES_MOONGATES,		// (SL) Define where the moongates are.
        SCRIPTOBJ_RES_MULTIDEF,                                           // compatibility    -> SCRIPTOBJ_RES_ITEMDEF
        SCRIPTOBJ_RES_NAMES,			// A block of possible names for a NPC type. (read as needed)
        SCRIPTOBJ_RES_NEWBIE,			// Triggers to execute on Player creation (based on skills selected)
        SCRIPTOBJ_RES_NOTOTITLES,		// (SI) Define the noto titles used.
        SCRIPTOBJ_RES_OBSCENE,		// (SL) A list of obscene words.
        SCRIPTOBJ_RES_PLEVEL,			// Define the list of commands that a PLEVEL can access. (or not access)
        SCRIPTOBJ_RES_REGIONRESOURCE,	// Define an Ore type.
        SCRIPTOBJ_RES_REGIONTYPE,		// Triggers etc. that can be assinged to a SCRIPTOBJ_RES_AREA
        SCRIPTOBJ_RES_RESOURCELIST,	// List of Sections to create lists from in DEFLIST
        SCRIPTOBJ_RES_RESOURCES,		// (SL) list of all the resource files we should index !
        SCRIPTOBJ_RES_ROOM,			// Non-complex region. (no extra tags)
        SCRIPTOBJ_RES_ROOMDEF,		// Non-complex region. (no extra tags)  // compatibility    -> SCRIPTOBJ_RES_ROOM
        SCRIPTOBJ_RES_RUNES,			// (SI) Define list of the magic runes.
        SCRIPTOBJ_RES_SCROLL,			// SCROLL_GUEST=message scroll sent to player at guest login. SCROLL_MOTD, SCROLL_NEWBIE
        SCRIPTOBJ_RES_SECTOR,			// Make changes to a sector. (SAVED in World)
        SCRIPTOBJ_RES_SERVERS,		// List a number of servers in 3 line format. (Phase this out)
        SCRIPTOBJ_RES_SKILL,			// Define attributes for a skill (how fast it raises etc)
        SCRIPTOBJ_RES_SKILLCLASS,		// Define specifics for a char with this skill class. (ex. skill caps)
        SCRIPTOBJ_RES_SKILLMENU,		// A menu that is attached to a skill. special arguments over other menus.
        SCRIPTOBJ_RES_SPAWN,			// Define a list of NPC's and how often they may spawn.
        SCRIPTOBJ_RES_SPEECH,			// A speech block with ON=*blah* in it.
        SCRIPTOBJ_RES_SPELL,			// Define a magic spell. (0-64 are reserved)
        SCRIPTOBJ_RES_SPHERE,			// Main Server INI block
        SCRIPTOBJ_RES_SPHERECRYPT,	// Encryption keys
        SCRIPTOBJ_RES_STARTS,			// (SI) List of starting locations for newbies.
        SCRIPTOBJ_RES_STAT,			// Stats elements like KARMA,STR,DEX,FOOD,FAME,CRIMINAL etc. Used for resource and desire scripts.
        SCRIPTOBJ_RES_TELEPORTERS,	// (SL) Where are the teleporters in the world ? dungeon transports etc.
        SCRIPTOBJ_RES_TEMPLATE,		// Define lists of items. (for filling loot etc)
        SCRIPTOBJ_RES_TIMERF,
        SCRIPTOBJ_RES_TIP,			// Tips (similar to SCRIPTOBJ_RES_SCROLL) that can come up at startup.
        SCRIPTOBJ_RES_TYPEDEF,		// Define a trigger block for a SCRIPTOBJ_RES_WORLDITEM m_type.
        SCRIPTOBJ_RES_TYPEDEFS,
        SCRIPTOBJ_RES_WC,				// =WORLDCHAR
        SCRIPTOBJ_RES_WEBPAGE,		// Define a web page template.
        SCRIPTOBJ_RES_WI,				// =WORLDITEM
        SCRIPTOBJ_RES_WORLDCHAR,		// Define instance of char in the world. (SAVED in World)
        SCRIPTOBJ_RES_WORLDITEM,		// Define instance of item in the world. (SAVED in World)
        SCRIPTOBJ_RES_WORLDLISTS,
        SCRIPTOBJ_RES_WORLDSCRIPT,	// Something to load into a script.
        SCRIPTOBJ_RES_WORLDVARS,
        SCRIPTOBJ_RES_WS,				// =WORLDSCRIPT
        SCRIPTOBJ_RES_QTY				// Don't care
    };

    static constexpr std::array<std::string_view, 1+SCRIPTOBJ_RES_QTY> resourceBlocks
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



    //Default Script Objects we have to deal with

    enum TAG_TYPE
    {
        SCRIPTOBJ_TAG_CATEGORY = 0,
        SCRIPTOBJ_TAG_COLOR,
        SCRIPTOBJ_TAG_DEFNAME,
        SCRIPTOBJ_TAG_DESCRIPTION,
        SCRIPTOBJ_TAG_DUPEITEM,
        SCRIPTOBJ_TAG_DUPELIST,
        SCRIPTOBJ_TAG_GROUP,
        SCRIPTOBJ_TAG_ID,
        SCRIPTOBJ_TAG_NAME,
        SCRIPTOBJ_TAG_SUBSECTION,
        SCRIPTOBJ_TAG_P,
        SCRIPTOBJ_TAG_POINT,
        SCRIPTOBJ_TAG_QTY
    };

    static constexpr std::array<std::string_view, 1+SCRIPTOBJ_TAG_QTY> objectTags
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

};


#endif // SCRIPTUTILS_H
