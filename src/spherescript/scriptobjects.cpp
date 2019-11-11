#include "scriptobjects.h"
#include "../globals.h"

/*--        Helper functions    --*/
static bool strEqual_DefaultCase( std::string const& str1, std::string const& str2 )
{
    if (g_settings.m_caseSensitiveScriptParsing)
        return (str1 == str2);
    return (0 == strcmpi(str1.c_str(), str2.c_str()));
}


/*--        ScriptObj           --*/

ScriptObj::ScriptObj() :
    m_type(SCRIPTOBJ_TYPE_NONE), m_category(nullptr), m_subsection(nullptr),
    m_description(SCRIPTOBJ_DESCRIPTION_NONE_NAME), m_name(SCRIPTOBJ_NAME_NONE),
    m_display(0), m_baseDef(false), m_color("0"), m_scriptFileIndex(-1), m_scriptLine(-1)
{
}

//ScriptObj::~ScriptObj()
//{
//}

/*--        ScriptSubsection        --*/

ScriptSubsection::ScriptSubsection(std::string subsectionName) :
    m_subsectionName(std::move(subsectionName))
{
}

ScriptSubsection::~ScriptSubsection()
{
    if (m_objects.empty())
        return;
    for (size_t i = m_objects.size() - 1; i > 0; --i)
        delete m_objects[i];
}



/*--        ScriptCategory          --*/

ScriptCategory::ScriptCategory(std::string categoryName) :
    m_categoryName(std::move(categoryName))
{
}

ScriptCategory::~ScriptCategory()
{
    if (m_subsections.empty())
        return;
    for (size_t i = m_subsections.size() - 1; i > 0; --i)
        delete m_subsections[i];
}

ScriptSubsection * ScriptCategory::findSubsection(const std::string &subsectionName, bool createNew)
{
    ScriptSubsection * subsection = nullptr;
    if ( ! m_subsections.empty() )
    {
        // looping by index should be slightly faster than looping by iterator
        for (ScriptSubsection* subsectionTest : m_subsections)
        {
            if (strEqual_DefaultCase(subsectionTest->m_subsectionName,subsectionName))
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

ScriptObjTree::~ScriptObjTree()
{
    if (m_categories.empty())
        return;
    for (size_t i = m_categories.size() - 1; i > 0; --i)
        delete m_categories[i];
}

ScriptCategory * ScriptObjTree::findCategory(const std::string& categoryName, bool createNew)
{
    ScriptCategory * category = nullptr;
    if ( ! m_categories.empty() )
    {
        for (ScriptCategory* categoryTest : m_categories)
        {
            if (strEqual_DefaultCase(categoryTest->m_categoryName, categoryName))
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


// Special iterators

ScriptObjTree::iterator ScriptObjTree::end()
{
    auto invalid = ScriptObjTree::iterator::kInvalidIdx;
    return {this, invalid, invalid, invalid};
}

ScriptObjTree::iterator ScriptObjTree::begin()
{
    if (!m_categories.empty())
    {
        ScriptCategory* firstCategory = m_categories[0];
        if (!firstCategory->m_subsections.empty())
        {
            ScriptSubsection* firstSubsection = firstCategory->m_subsections[0];
            if (!firstSubsection->m_objects.empty())
            {
                return {this, 0, 0, 0};
            }
        }
    }
    return end();
}

ScriptObjTree::iterator ScriptObjTree::back_it() // last element
{
    if (m_categories.empty())
        return end();

    size_t lastCategoryIdx = m_categories.size() - 1;
    ScriptCategory* lastCategory = m_categories[lastCategoryIdx];
    if (lastCategory->m_subsections.empty())
        return end();

    size_t lastSubsectionIdx = lastCategory->m_subsections.size() - 1;
    ScriptSubsection* lastSubsection = lastCategory->m_subsections[lastSubsectionIdx];
    if (lastSubsection->m_objects.empty())
        return end();

    size_t lastObjectIdx = lastSubsection->m_objects.size() - 1;

    return {this, lastCategoryIdx, lastSubsectionIdx, lastObjectIdx};
}

ScriptObjTree::const_iterator ScriptObjTree::cend() const
{
    auto invalid = ScriptObjTree::const_iterator::kInvalidIdx;
    return {this, invalid, invalid, invalid};
}

ScriptObjTree::const_iterator ScriptObjTree::cbegin() const
{
    if (!m_categories.empty())
    {
        ScriptCategory* firstCategory = m_categories[0];
        if (!firstCategory->m_subsections.empty())
        {
            ScriptSubsection* firstSubsection = firstCategory->m_subsections[0];
            if (!firstSubsection->m_objects.empty())
            {
                return {this, 0, 0, 0};
            }
        }
    }
    return cend();
}

ScriptObjTree::const_iterator ScriptObjTree::cback_it() const // last element
{
    if (m_categories.empty())
        return cend();

    size_t lastCategoryIdx = m_categories.size() - 1;
    ScriptCategory* lastCategory = m_categories[lastCategoryIdx];
    if (lastCategory->m_subsections.empty())
        return cend();

    size_t lastSubsectionIdx = lastCategory->m_subsections.size() - 1;
    ScriptSubsection* lastSubsection = lastCategory->m_subsections[lastSubsectionIdx];
    if (lastSubsection->m_objects.empty())
        return cend();

    size_t lastObjectIdx = lastSubsection->m_objects.size() - 1;

    return {this, lastCategoryIdx, lastSubsectionIdx, lastObjectIdx};
}
