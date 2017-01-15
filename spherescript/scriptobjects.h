#ifndef SCRIPTOBJECTS_H
#define SCRIPTOBJECTS_H

#include <string>
#include <vector>


// Resource types managed by Leviathan.
#define SCRIPTOBJ_TYPE_NONE 0
#define SCRIPTOBJ_TYPE_ITEM 1
#define SCRIPTOBJ_TYPE_CHAR 2
#define SCRIPTOBJ_TYPE_DEF 3
#define SCRIPTOBJ_TYPE_AREA 4
#define SCRIPTOBJ_TYPE_SPAWN 5
#define SCRIPTOBJ_TYPE_TEMPLATE 6
#define SCRIPTOBJ_TYPE_SPELL 7
#define SCRIPTOBJ_TYPE_MULTI 8

#define SCRIPTOBJ_NAME_NONE             "<No Name>"
#define SCRIPTOBJ_DESCRIPTION_NONE_NAME "<No Description>"
#define SCRIPTCATEGORY_NONE_NAME        "<No Category>"
#define SCRIPTSUBSECTION_NONE_NAME      "<No Subsection>"


class ScriptCategory;
class ScriptSubsection;

class ScriptObj
{
public:
    ScriptObj();
    ~ScriptObj();
    //void writeBlock(std::ifstream &fileStream);
    char m_type;
    ScriptCategory *m_category;
    ScriptSubsection *m_subsection;
    std::string m_description;
    std::string m_name;
    int m_ID;
    std::string m_defname;
    std::string m_dupeItem;
    unsigned short m_color;
    std::string m_display;
    std::string m_scriptFile;
    int m_scriptLine;           // line in the script file where the [*DEF] block starts
};

class ScriptSubsection
{
public:
    ScriptSubsection(std::string subsectionName);
    ~ScriptSubsection();
    std::string m_subsectionName;
    std::vector<ScriptObj*> m_objects;
    ScriptCategory *m_category = nullptr;
};

class ScriptCategory
{
public:
    ScriptCategory(std::string categoryName);
    ~ScriptCategory();
    std::string m_categoryName;
    std::vector<ScriptSubsection*> m_subsections;
    ScriptSubsection *findSubsection(std::string subsectionName, bool createNew = true);
};

class ScriptObjTree
{
public:
    //ScriptObjTree();
    ~ScriptObjTree();
    std::vector<ScriptCategory*> m_categories;
    ScriptCategory *findCategory(std::string categoryName, bool createNew = true); // createNew: create a new category if one named categoryName doesn't exist.
};


#endif // SCRIPTOBJECTS_H
