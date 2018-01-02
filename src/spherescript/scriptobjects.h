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

// Not using a define macro because in this way i can pass the arguments for
//  findCategory and findSubsection by reference
const std::string SCRIPTOBJ_NAME_NONE             = "<No Name>";
const std::string SCRIPTOBJ_DESCRIPTION_NONE_NAME = "<No Description>";
const std::string SCRIPTCATEGORY_NONE_NAME        = "<No Category>";
const std::string SCRIPTSUBSECTION_NONE_NAME      = "<No Subsection>";


class ScriptCategory;
class ScriptSubsection;

class ScriptObj
{
public:
    ScriptObj();
    //~ScriptObj();
    //void writeBlock(std::ifstream &fileStream);
    char m_type;
    ScriptCategory *m_category;
    ScriptSubsection *m_subsection;
    std::string m_description;
    std::string m_name;
    std::string m_ID;
    std::string m_defname;
    int m_display;          // ID to display
    bool m_baseDef;         // Is this a base or a derived char/itemdef? (so is this item body inherited from another chardef?)
    std::string m_dupeItem;
    std::string m_color;
    int m_scriptFileIndex;
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
    ScriptSubsection *findSubsection(const std::string &subsectionName, bool createNew = true);
};

class ScriptObjTree
{
public:
    //ScriptObjTree();
    ~ScriptObjTree();
    std::vector<ScriptCategory*> m_categories;
    ScriptCategory *findCategory(const std::string &categoryName, bool createNew = true); // createNew: create a new category if one named categoryName doesn't exist.

    class iterator;
    iterator end();         // invalid iterator (obtained when incrementing an iterator to the last item)
    iterator begin();       // iterator to first item
    iterator back_it();     // iterator to last item
};

class ScriptObjTree::iterator
{
    friend iterator ScriptObjTree::end();
    friend iterator ScriptObjTree::begin();
    friend iterator ScriptObjTree::back_it();
private:
    ScriptObjTree* m_parentTree;
    size_t m_currentCategoryIdx;
    size_t m_currentSubsectionIdx;
    size_t m_currentObjectIdx;

public:
    iterator(); // constructs an invalid iterator
    iterator(ScriptObjTree* parentTree, size_t currentCategoryIdx, size_t currentSubsectionIdx, size_t currentObjectIdx);
    void operator=(const iterator& source);
    bool operator==(const iterator& toCmp) const;
    bool operator!=(const iterator& toCmp) const;
    iterator operator++();      // pre-increment
    iterator operator++(int);   // post-increment
    iterator operator--();      // pre-decrement
    iterator operator--(int);   // post-decrement
    ScriptObj* operator*();

    static const size_t kInvalidIdx = (size_t)-1;
};

#endif // SCRIPTOBJECTS_H