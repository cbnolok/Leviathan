#include "scriptsearch.h"
#include "../cpputils.h"
#include <cstring>

ScriptSearch::ScriptSearch
    (const std::vector<ScriptObjTree *> &trees, SearchData_t data) :
    m_trees(trees), m_searchBy(data.searchBy), m_caseSensitive(data.caseSensitive), m_key(data.key),
    m_curTreeIdx(0), m_it(m_trees[0]->begin()), m_lastFoundTreeIdx(0), m_it_lastFound(m_trees[0]->end()),
    m_lastOperation(LastOperation_t::None)
{
    if (!data.caseSensitive)
        strToUpper(m_key);
}

ScriptObj* ScriptSearch::isMatch()
{
    ScriptObj* obj = *m_it;

    std::string tmp;
    if ( m_searchBy == SearchBy_t::ID )
        tmp = obj->m_ID;
    else if ( m_searchBy == SearchBy_t::Defname )
        tmp = obj->m_defname;
    else if ( m_searchBy == SearchBy_t::Description )
        tmp = obj->m_description;

    if (!m_caseSensitive)
        strToUpper(tmp);


    if (tmp.find(m_key) != std::string::npos)
        return obj;
    else
        return nullptr;
}

ScriptObj* ScriptSearch::next()
{
    if (m_it == m_trees[ m_curTreeIdx ]->end())
    {
        if (m_it_lastFound != m_trees[ m_curTreeIdx ]->end() )
        {
            m_curTreeIdx = m_lastFoundTreeIdx;
            m_it = m_it_lastFound;
        }
    }
    if (m_lastOperation != LastOperation_t::None)
        ++m_it;

    while (*m_it)
    {
        ScriptObj* result = isMatch();
        if (result)
        {
            m_lastOperation = LastOperation_t::Next;
            m_lastFoundTreeIdx = m_curTreeIdx;
            m_it_lastFound = m_it;
            return result;
        }
        ++m_it;
    }
    if (m_curTreeIdx < m_trees.size() - 1)  // done with a tree, search into another one?
    {
        m_it = m_trees[ ++m_curTreeIdx ]->begin();
        return next();
    }
    return nullptr;
}

ScriptObj* ScriptSearch::previous()
{
    if (m_it == m_trees[ m_curTreeIdx ]->end())
    {
        if (m_it_lastFound != m_trees[ m_curTreeIdx ]->end() )
        {
            m_curTreeIdx = m_lastFoundTreeIdx;
            m_it = m_it_lastFound;
        }
    }
    if (m_lastOperation != LastOperation_t::None)
        --m_it;

    while (*m_it)
    {
        ScriptObj* result = isMatch();
        if (result)
        {
            m_lastOperation = LastOperation_t::Previous;
            m_lastFoundTreeIdx = m_curTreeIdx;
            m_it_lastFound = m_it;
            return result;
        }
        --m_it;
    }
    if (m_curTreeIdx > 0)   // done with a tree, search into another one?
    {
        m_it = m_trees[ --m_curTreeIdx ]->back_it();
        return previous();
    }
    return nullptr;
}
