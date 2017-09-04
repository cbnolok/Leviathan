#include "globals.h"
#include "spherescript/scriptobjects.h"
#include "uofiles/uohues.h"
#include "uofiles/uoart.h"
#include "uofiles/uoanim.h"
#include <QApplication>


Settings g_settings;

int g_loadedClientProfile = -1;
std::vector<ClientProfile> g_clientProfiles;

int g_loadedScriptsProfile = -1;
std::vector<ScriptsProfile> g_scriptsProfiles;
std::vector<std::string> g_scriptFileList;

ScriptObjTree *g_scriptObjTree_Chars        = nullptr;
ScriptObjTree *g_scriptObjTree_Spawns       = nullptr;
ScriptObjTree *g_scriptObjTree_Items        = nullptr;
ScriptObjTree *g_scriptObjTree_Templates    = nullptr;
ScriptObjTree *g_scriptObjTree_Defs         = nullptr;
ScriptObjTree *g_scriptObjTree_Areas        = nullptr;
ScriptObjTree *g_scriptObjTree_Spells       = nullptr;
ScriptObjTree *g_scriptObjTree_Multis       = nullptr;

ScriptObjTree * objTree(int objType)
{
    switch (objType)
    {
    case SCRIPTOBJ_TYPE_ITEM:       return g_scriptObjTree_Items;       break;
    case SCRIPTOBJ_TYPE_CHAR:       return g_scriptObjTree_Chars;       break;
    case SCRIPTOBJ_TYPE_DEF:        return g_scriptObjTree_Defs;        break;
    case SCRIPTOBJ_TYPE_AREA:       return g_scriptObjTree_Areas;       break;
    case SCRIPTOBJ_TYPE_SPAWN:      return g_scriptObjTree_Spawns;      break;
    case SCRIPTOBJ_TYPE_TEMPLATE:   return g_scriptObjTree_Templates;   break;
    case SCRIPTOBJ_TYPE_SPELL:      return g_scriptObjTree_Spells;      break;
    case SCRIPTOBJ_TYPE_MULTI:      return g_scriptObjTree_Multis;      break;
    default:    return nullptr;
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


/* Client files stuff */

UOHues *g_UOHues = nullptr;
UOArt  *g_UOArt  = nullptr;
UOAnim *g_UOAnim = nullptr;

void loadClientFiles()
{
    if (g_loadedClientProfile == -1)
        return;

    delete g_UOHues;
    delete g_UOArt;
    delete g_UOAnim;

    std::string& clientFolder = g_clientProfiles[g_loadedClientProfile].m_clientPath;

    appendToLog("Loading Client Profile \"" + g_clientProfiles[g_loadedClientProfile].m_name + "\"...");

    QApplication::processEvents();
    g_UOHues = new UOHues(clientFolder + "hues.mul");
    QApplication::processEvents();
    g_UOArt  = new UOArt (clientFolder);
    QApplication::processEvents();
    g_UOAnim = new UOAnim(clientFolder);
    QApplication::processEvents();

    appendToLog("Client Profile \"" + g_clientProfiles[g_loadedClientProfile].m_name + "\" loaded.");
}

