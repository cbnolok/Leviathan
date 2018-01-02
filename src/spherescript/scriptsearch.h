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

    enum class LastOperation_t
    {
        None,
        Next,
        Previous
    };

    typedef struct SearchData_s // to comfortably pass the data relating to a search session
    {
        SearchBy_t      searchBy;
        bool            caseSensitive;
        std::string     key;

        bool            initialized;    // needed by SubDlg_SearchObj
        SearchData_s(): initialized(false) {}
    } SearchData_t;


public:
    ScriptSearch(const std::vector<ScriptObjTree *> &trees, SearchData_t data);
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
    LastOperation_t m_lastOperation;

    ScriptObj *isMatch();
};

#endif // SCRIPTSEARCH_H
