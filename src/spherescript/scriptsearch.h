#ifndef SCRIPTSEARCH_H
#define SCRIPTSEARCH_H

#include "scriptobjects.h"
#include <memory>


struct ScriptSearch
{
    enum class SearchBy
    {
        ID,
        Defname,
        Description
    };

    enum class LastOperation
    {
        None,
        Next,
        Previous
    };

    struct SearchData // to comfortably pass the data relating to a search session
    {
        SearchBy        searchBy;
        bool            caseSensitive;
        std::string     key;
    };


public:
    ScriptSearch(const std::vector<std::unique_ptr<ScriptObjTree> *> trees, SearchData data = {});
    ScriptObj* next();
    ScriptObj* previous();

private:
    std::vector<std::unique_ptr<ScriptObjTree> *> m_trees;
    SearchBy m_searchBy;
    bool m_caseSensitive;
    std::string m_key;

    size_t m_curTreeIdx;
    ScriptObjTree::iterator m_it;
    size_t m_lastFoundTreeIdx;
    ScriptObjTree::iterator m_it_lastFound;
    LastOperation m_lastOperation;

    ScriptObj *isMatch();
};

#endif // SCRIPTSEARCH_H
