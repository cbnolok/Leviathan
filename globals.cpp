#include "globals.h"

int g_loadedScriptsProfile;
std::vector<ScriptsProfile> g_scriptsProfiles;

ScriptObjTree *g_scriptObjTree_Chars = nullptr;
ScriptObjTree *g_scriptObjTree_Spawns = nullptr;
ScriptObjTree *g_scriptObjTree_Items = nullptr;
ScriptObjTree *g_scriptObjTree_Templates = nullptr;
ScriptObjTree *g_scriptObjTree_Defs = nullptr;
ScriptObjTree *g_scriptObjTree_Areas = nullptr;
ScriptObjTree *g_scriptObjTree_Spells = nullptr;
ScriptObjTree *g_scriptObjTree_Multis = nullptr;

ScriptObjTree * objTree(int objType)
{
    switch (objType)
    {
    case SCRIPTOBJ_TYPE_ITEM:
        return g_scriptObjTree_Items;
    case SCRIPTOBJ_TYPE_CHAR:
        return g_scriptObjTree_Chars;
    case SCRIPTOBJ_TYPE_DEF:
        return g_scriptObjTree_Defs;
    case SCRIPTOBJ_TYPE_AREA:
        return g_scriptObjTree_Areas;
    case SCRIPTOBJ_TYPE_SPAWN:
        return g_scriptObjTree_Spawns;
    case SCRIPTOBJ_TYPE_TEMPLATE:
        return g_scriptObjTree_Templates;
    case SCRIPTOBJ_TYPE_SPELL:
        return g_scriptObjTree_Spells;
    case SCRIPTOBJ_TYPE_MULTI:
        return g_scriptObjTree_Multis;
    default:
        return nullptr;
    }
}


/*  Log stuff   */

// The tab_log (which is a QWidget class) instance must be constructed after the main application,
//  so i can't do it here. It's done in mainwindow.cpp.
MainTab_Log *g_MainTab_Log_inst = nullptr;

bool appendToLog(const char *str)
{
    if (g_MainTab_Log_inst == nullptr)
        return false;
    g_MainTab_Log_inst->appendText(str);
    return true;
}

bool appendToLog(const std::string &str)
{
    return appendToLog(str.c_str());
}
