#include "scriptobjects.h"
#include "../globals.h"


/*--        ScriptObj           --*/

ScriptObj::ScriptObj() :
    m_type(SCRIPTOBJ_TYPE_NONE), m_category(nullptr), m_subsection(nullptr), m_description("<No Description>"), m_name("<No Name>"),
    m_ID(0), m_defname("<No Defname>"), m_dupeItem(""), m_color(0), m_display(""), m_scriptFile("<No ScriptFile>"), m_scriptLine(-1)
{
}

ScriptObj::~ScriptObj()
{
}

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

ScriptSubsection * ScriptCategory::findSubsection(std::string subsectionName, bool createNew)
{
    ScriptSubsection * subsection = nullptr;
    if ( ! m_subsections.empty() )
    {
        std::vector<ScriptSubsection*>::iterator it;
        for (it = m_subsections.begin(); it != m_subsections.end(); it++)
        {
            ScriptSubsection * subsectionTest = std::addressof(**it);  // using std::addressof to avoid problems whether i'll decide to overload & operator.
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

ScriptCategory * ScriptObjTree::findCategory(std::string categoryName, bool createNew)
{
    ScriptCategory * category = nullptr;
    if ( ! m_categories.empty() )
    {
        std::vector<ScriptCategory*>::iterator it;
        for (it = m_categories.begin(); it != m_categories.end(); it++)
        {
            ScriptCategory * categoryTest = std::addressof(**it);  // using std::addressof to avoid problems whether i'll decide to overload & operator.
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

