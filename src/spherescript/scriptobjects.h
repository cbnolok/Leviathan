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
    ScriptObjTree() = default;
    ~ScriptObjTree();
    std::vector<ScriptCategory*> m_categories;
    ScriptCategory *findCategory(const std::string &categoryName, bool createNew = true); // createNew: create a new category if one named categoryName doesn't exist.

    // Iterators
    template <typename PointerType> class base_iterator;
    using iterator = base_iterator<ScriptObj*>;
    iterator end();         // invalid iterator (obtained when incrementing an iterator to the last item)
    iterator begin();       // iterator to first item
    iterator back_it();     // iterator to last item
    using const_iterator = base_iterator<const ScriptObj*>;
    const_iterator cend() const;
    const_iterator cbegin() const;
    const_iterator cback_it() const;
};

template <typename PointerType>
class ScriptObjTree::base_iterator
{
private:
    const ScriptObjTree* m_parentTree;
    size_t m_currentCategoryIdx;
    size_t m_currentSubsectionIdx;
    size_t m_currentObjectIdx;

public:
    base_iterator() = delete; // would construct an invalid iterator
    base_iterator(const ScriptObjTree* parentTree, size_t currentCategoryIdx, size_t currentSubsectionIdx, size_t currentObjectIdx);
    ~base_iterator() = default;
    base_iterator(const base_iterator&) = default;                // copy constructor
    base_iterator(base_iterator&&) noexcept = default;            // move constructor
    base_iterator& operator=(const base_iterator&) = default;     // copy assignment operator
    base_iterator& operator=(base_iterator&&) noexcept = default; // move assignment operator
    bool operator==(const base_iterator&) const;
    bool operator!=(const base_iterator&) const;
    base_iterator operator++();      // pre-increment
    base_iterator operator++(int);   // post-increment
    base_iterator operator--();      // pre-decrement
    base_iterator operator--(int);   // post-decrement
    PointerType operator*();

    static const size_t kInvalidIdx = (size_t)-1;
};



/*--        ScriptObjTree::iterator           --*/

template <typename T>
ScriptObjTree::base_iterator<T>::base_iterator(const ScriptObjTree *parentTree, size_t currentCategoryIdx, size_t currentSubsectionIdx, size_t currentObjectIdx) :
    m_parentTree(parentTree),
    m_currentCategoryIdx(currentCategoryIdx), m_currentSubsectionIdx(currentSubsectionIdx), m_currentObjectIdx(currentObjectIdx)
{
}

template <typename T>
bool ScriptObjTree::base_iterator<T>::operator==(const base_iterator& toCmp) const
{
    return ( (m_parentTree == toCmp.m_parentTree) &&
         (m_currentCategoryIdx == toCmp.m_currentCategoryIdx) &&
         (m_currentSubsectionIdx == toCmp.m_currentSubsectionIdx) &&
         (m_currentObjectIdx == toCmp.m_currentObjectIdx) );
}

template <typename T>
bool ScriptObjTree::base_iterator<T>::operator!=(const base_iterator& toCmp) const
{
    return ( (m_parentTree != toCmp.m_parentTree) ||
         (m_currentCategoryIdx != toCmp.m_currentCategoryIdx) ||
         (m_currentSubsectionIdx != toCmp.m_currentSubsectionIdx) ||
         (m_currentObjectIdx != toCmp.m_currentObjectIdx) );
}

template <typename T>
ScriptObjTree::base_iterator<T> ScriptObjTree::base_iterator<T>::operator++()   // pre-increment
{
    ScriptCategory* curCategory = m_parentTree->m_categories[m_currentCategoryIdx];
    ScriptSubsection* curSubsection = curCategory->m_subsections[m_currentSubsectionIdx];
    //ScriptObj& curObj = curSubsection[m_currentObjIdx];
    if (m_currentObjectIdx < curSubsection->m_objects.size() - 1)
        ++ m_currentObjectIdx;
    else
    {
        m_currentObjectIdx = 0;
        if (m_currentSubsectionIdx < curCategory->m_subsections.size() - 1)
            ++ m_currentSubsectionIdx;
        else
        {
            m_currentSubsectionIdx = 0;
            if (m_currentCategoryIdx < m_parentTree->m_categories.size() - 1)
                ++ m_currentCategoryIdx;
            else
            {
                // "end" iterator
                m_currentObjectIdx = kInvalidIdx;
                m_currentSubsectionIdx = kInvalidIdx;
                m_currentCategoryIdx = kInvalidIdx;
            }
        }
    }
    return *this;
}

template <typename T>
ScriptObjTree::base_iterator<T> ScriptObjTree::base_iterator<T>::operator++(int) // post-increment
{
    iterator oldIt = *this;
    ++(*this);  // do pre-increment
    return oldIt;
}

template <typename T>
ScriptObjTree::base_iterator<T> ScriptObjTree::base_iterator<T>::operator--()   // pre-decrement
{
    if (m_currentObjectIdx > 0)
        -- m_currentObjectIdx;
    else
    {
        if (m_currentSubsectionIdx > 0)
            -- m_currentSubsectionIdx;
        else
        {
            if (m_currentCategoryIdx > 0)
                -- m_currentCategoryIdx;
            else
            {
                // "end" iterator
                m_currentObjectIdx = kInvalidIdx;
                m_currentSubsectionIdx = kInvalidIdx;
                m_currentCategoryIdx = kInvalidIdx;
                return *this;
            }
            // set the current sebsection index to the last subsection of the active category
            m_currentSubsectionIdx = m_parentTree->m_categories[m_currentCategoryIdx]->m_subsections.size() - 1;
        }
        // set the current object index to the last object of the active subsection of the active category
        m_currentObjectIdx = m_parentTree->m_categories[m_currentCategoryIdx]->m_subsections[m_currentSubsectionIdx]->m_objects.size() - 1;
    }
    return *this;
}

template <typename T>
ScriptObjTree::base_iterator<T> ScriptObjTree::base_iterator<T>::operator--(int) // post-decrement
{
    iterator oldIt = iterator(m_parentTree, m_currentCategoryIdx, m_currentSubsectionIdx, m_currentObjectIdx);
   --(*this);  // do pre-decrement
    return oldIt;
}

template <typename PointerType>
PointerType ScriptObjTree::base_iterator<PointerType>::operator*()
{
    if (m_parentTree == nullptr)
        return nullptr;
    if ( (m_currentCategoryIdx == kInvalidIdx) || (m_currentSubsectionIdx == kInvalidIdx) || (m_currentObjectIdx == kInvalidIdx) )
        return nullptr;

    return m_parentTree->
            m_categories[m_currentCategoryIdx]->
            m_subsections[m_currentSubsectionIdx]->
            m_objects[m_currentObjectIdx];
}

#endif // SCRIPTOBJECTS_H
