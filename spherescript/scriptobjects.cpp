#include "scriptobjects.h"
#include "../globals.h"


/*--        ScriptObj           --*/

ScriptObj::ScriptObj() :
    m_type(SCRIPTOBJ_TYPE_NONE), m_category(nullptr), m_subsection(nullptr),
    m_description(SCRIPTOBJ_DESCRIPTION_NONE_NAME), m_name(SCRIPTOBJ_NAME_NONE),
    m_display(0), m_baseDef(false), m_color("0"), m_scriptLine(-1)
{
}

//ScriptObj::~ScriptObj()
//{
//}

/*--        ScriptSubsection        --*/

ScriptSubsection::ScriptSubsection(std::string subsectionName) :
    m_subsectionName(subsectionName)
{
}

ScriptSubsection::~ScriptSubsection()
{
    if (!m_objects.size())
        return;
    for (size_t i = m_objects.size()-1; i > 0; i--)
        delete m_objects[i];
}



/*--        ScriptCategory          --*/

ScriptCategory::ScriptCategory(std::string categoryName) :
    m_categoryName(categoryName)
{
}

ScriptCategory::~ScriptCategory()
{
    if (!m_subsections.size())
        return;
    for (size_t i = m_subsections.size()-1; i > 0; i--)
        delete m_subsections[i];
}

ScriptSubsection * ScriptCategory::findSubsection(const std::string &subsectionName, bool createNew)
{
    ScriptSubsection * subsection = nullptr;
    if ( ! m_subsections.empty() )
    {
        // looping by index should be slightly faster than looping by iterator
        for (size_t i = 0, end = m_subsections.size(); i < end; ++i)
        {
            ScriptSubsection * subsectionTest = m_subsections[i];
            if (subsectionTest->m_subsectionName == subsectionName)
            {
                subsection = subsectionTest;
                break;
            }
        }
    }
    if ( (subsection == nullptr) && createNew )
    {
        // Subsection didn't exist inside this Category.  Create a new one.
        subsection = new ScriptSubsection(subsectionName);
        m_subsections.push_back(subsection);  // insert the category in the script object tree
    }
    return subsection;
}



/*--        ScriptObjTree           --*/

/*
ScriptObjTree::ScriptObjTree()
{
}
*/


ScriptObjTree::~ScriptObjTree()
{
    if (!m_categories.size())
        return;
    for (size_t i = m_categories.size()-1; i > 0; i--)
        delete m_categories[i];
}

ScriptCategory * ScriptObjTree::findCategory(const std::string& categoryName, bool createNew)
{
    ScriptCategory * category = nullptr;
    if ( ! m_categories.empty() )
    {
        for (size_t i = 0, end = m_categories.size(); i < end; ++i)
        {
            ScriptCategory * categoryTest = m_categories[i];
            if (categoryTest->m_categoryName == categoryName)
            {
                category = categoryTest;
                break;
            }
        }
    }
    if ( (category == nullptr) && createNew )
    {
        // Category didn't exist.  Create a new one.
        category = new ScriptCategory(categoryName);
        m_categories.push_back(category);  // insert the category in the script object tree
    }
    return category;
}


ScriptObjTree::iterator ScriptObjTree::begin()
{
    if (m_categories.size() > 0)
    {
        ScriptCategory* firstCategory = m_categories[0];
        if (firstCategory->m_subsections.size() > 0)
        {
            ScriptSubsection* firstSubsection = firstCategory->m_subsections[0];
            if (firstSubsection->m_objects.size() > 0)
            {
                //ScriptObj& firstSubsection.m_objects[0];
                return iterator(this, 0, 0, 0);
            }
        }
    }
    return iterator();  // invalid iterator
}

ScriptObjTree::iterator ScriptObjTree::end()
{
    if (m_categories.size() == 0)
        return iterator();  // invalid iterator

    size_t lastCategoryIdx = m_categories.size() - 1;
    ScriptCategory* lastCategory = m_categories[lastCategoryIdx];
    if (lastCategory->m_subsections.size() == 0)
        return iterator();

    size_t lastSubsectionIdx = lastCategory->m_subsections.size() - 1;
    ScriptSubsection* lastSubsection = lastCategory->m_subsections[lastSubsectionIdx];
    if (lastSubsection->m_objects.size() == 0)
        return iterator();

    size_t lastObjectIdx = lastSubsection->m_objects.size() - 1;

    return iterator(this, lastCategoryIdx, lastSubsectionIdx, lastObjectIdx);
}


/*--        ScriptObjTree::iterator           --*/


ScriptObjTree::iterator::iterator(ScriptObjTree* parentTree, size_t currentCategoryIdx, size_t currentSubsectionIdx, size_t currentObjectIdx) :
    m_parentTree(parentTree),
    m_currentCategoryIdx(currentCategoryIdx), m_currentSubsectionIdx(currentSubsectionIdx), m_currentObjectIdx(currentObjectIdx)
{
}

ScriptObjTree::iterator::iterator() :
    m_parentTree(nullptr), m_currentCategoryIdx(kInvalidIdx), m_currentSubsectionIdx(kInvalidIdx), m_currentObjectIdx(kInvalidIdx)
{
}

bool ScriptObjTree::iterator::operator==(iterator& toCmp) const
{
    if ( (m_currentCategoryIdx == toCmp.m_currentCategoryIdx) &&
         (m_currentSubsectionIdx == toCmp.m_currentSubsectionIdx) &&
         (m_currentObjectIdx == toCmp.m_currentObjectIdx) )
        return true;
    else
        return false;
}

bool ScriptObjTree::iterator::operator!=(iterator& toCmp) const
{
    if ( *this == toCmp )
        return false;
    else
        return true;
}

ScriptObjTree::iterator ScriptObjTree::iterator::operator++()   // pre-increment
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
                m_currentObjectIdx = kInvalidIdx;
                m_currentSubsectionIdx = kInvalidIdx;
                m_currentCategoryIdx = kInvalidIdx;
                m_parentTree = nullptr;
            }
        }
    }
    return *this;
}

ScriptObjTree::iterator ScriptObjTree::iterator::operator++(int) // post-increment
{
    iterator oldIt = iterator(m_parentTree, m_currentCategoryIdx, m_currentSubsectionIdx, m_currentObjectIdx);
    ++(*this);  // do pre-increment
    return oldIt;
}

ScriptObj* ScriptObjTree::iterator::operator*()
{
    if (m_parentTree == nullptr)
        return nullptr;

    return m_parentTree->
            m_categories[m_currentCategoryIdx]->
            m_subsections[m_currentSubsectionIdx]->
            m_objects[m_currentObjectIdx];
}

