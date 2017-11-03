#ifndef SCRIPTSEARCH_H
#define SCRIPTSEARCH_H

#include "scriptobjects.h"
#include <string>
#include <vector>


struct ScriptSearch
{
    enum class SearchBy_t
    {
        ID,
        Defname,
        Description
    };

    /*
    typedef struct SearchData_s
    {
        SearchBy_t searchBy;
        bool caseSensitive;
        std::string key;
    } SearchData_t;
    */

public:
    ScriptSearch(const std::vector<ScriptObjTree *> &trees, SearchBy_t searchBy, bool caseSensitive, std::string key);
    ScriptObj* next();
    ScriptObj* previous();

private:
    std::vector<ScriptObjTree *> m_trees;
    SearchBy_t m_searchBy;
    bool m_caseSensitive;
    std::string m_key;

    size_t m_curTreeIdx;
    ScriptObjTree::iterator m_it;
    size_t m_lastFoundTreeIdx;
    ScriptObjTree::iterator m_it_lastFound;

    enum class LastOperation_t
    {
        None,
        Next,
        Previous
    } m_lastOperation;

    ScriptObj *isMatch();
};

#endif // SCRIPTSEARCH_H
