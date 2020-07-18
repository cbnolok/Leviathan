#include "globals.h"
#include "logging.h"
#include "spherescript/scriptobjects.h"
#include "uoclientfiles/exceptions.h"
#include "uoclientfiles/uoart.h"
#include "uoclientfiles/uoanim.h"
#include "uoclientfiles/uohues.h"
#include "uoclientfiles/uomap.h"
#include "uoclientfiles/uostatics.h"
#include "uoclientfiles/uoradarcol.h"


AppSettings g_settings;

int g_loadedClientProfile = -1;
std::vector<ClientProfile> g_clientProfiles;

int g_loadedScriptsProfile = -1;
std::vector<ScriptsProfile> g_scriptsProfiles;
std::vector<std::string> g_scriptFileList;

bool g_sendKeystrokeAndFocusClient = true;

ScriptObjTree *g_scriptObjTree_Chars        = nullptr;
ScriptObjTree *g_scriptObjTree_Spawns       = nullptr;
ScriptObjTree *g_scriptObjTree_Items        = nullptr;
ScriptObjTree *g_scriptObjTree_Templates    = nullptr;
ScriptObjTree *g_scriptObjTree_Defs         = nullptr;
ScriptObjTree *g_scriptObjTree_Areas        = nullptr;
ScriptObjTree *g_scriptObjTree_Spells       = nullptr;
ScriptObjTree *g_scriptObjTree_Multis       = nullptr;

ScriptObjTree * getScriptObjTree(int objType)
{
    switch (objType)
    {
    case SCRIPTOBJ_TYPE_ITEM:       return g_scriptObjTree_Items;
    case SCRIPTOBJ_TYPE_CHAR:       return g_scriptObjTree_Chars;
    case SCRIPTOBJ_TYPE_DEF:        return g_scriptObjTree_Defs;
    case SCRIPTOBJ_TYPE_AREA:       return g_scriptObjTree_Areas;
    case SCRIPTOBJ_TYPE_SPAWN:      return g_scriptObjTree_Spawns;
    case SCRIPTOBJ_TYPE_TEMPLATE:   return g_scriptObjTree_Templates;
    case SCRIPTOBJ_TYPE_SPELL:      return g_scriptObjTree_Spells;
    case SCRIPTOBJ_TYPE_MULTI:      return g_scriptObjTree_Multis;
    default:    return nullptr;
    }
}


/*  Log stuff   */

LogEventEmitter g_logEventEmitter;
void appendToLog(const std::string &str)
{
    g_logEventEmitter.append(str);
}


/* Client files stuff */

uocf::UOArt         *g_UOArt        = nullptr;
uocf::UOAnim        *g_UOAnim       = nullptr;
uocf::UOHues        *g_UOHues       = nullptr;
uocf::UORadarCol    *g_UORadarCol   = nullptr;
std::vector<uocf::UOMap *> g_UOMaps;
std::vector<uocf::UOStatics *> g_UOStatics;

void loadClientFiles(std::function<void(int)> reportProgress)
{
    if (g_loadedClientProfile == -1)
        return;

    delete g_UOHues;
    delete g_UOArt;
    delete g_UOAnim;

    const std::string& clientFolder = g_clientProfiles[g_loadedClientProfile].m_clientPath;

    appendToLog("Loading Client Profile \"" + g_clientProfiles[g_loadedClientProfile].m_name + "\"...");

    g_UOHues        = new uocf::UOHues(clientFolder + "hues.mul");
    g_UORadarCol    = new uocf::UORadarCol(clientFolder + "radarcol.mul");
    g_UOArt         = new uocf::UOArt (clientFolder);
    g_UOAnim        = new uocf::UOAnim(clientFolder, reportProgress);

    g_UOMaps.resize(uocf::UOMap::kMaxSupportedMap + 1);
    g_UOStatics.resize(uocf::UOMap::kMaxSupportedMap + 1);
    for (unsigned i = 0; i <= uocf::UOMap::kMaxSupportedMap; ++i)
    {
        try
        {
            g_UOMaps[i] = new uocf::UOMap(clientFolder, i);
        }
        catch (uocf::InvalidStreamException&)
        {
            g_UOMaps[i] = nullptr;
            appendToLog("Can't open map" + std::to_string(i) + ".mul.");
            continue;
        }
        catch (uocf::MalformedFileException&)
        {
            g_UOMaps[i] = nullptr;
            appendToLog("Invalid size for map" + std::to_string(i) + ".mul.");
            continue;
        }
        //catch (UnsupportedActionException&)

        try
        {
            g_UOStatics[i] = new uocf::UOStatics(clientFolder, i, g_UOMaps[i]->getWidth(), g_UOMaps[i]->getHeight());
        }
        catch (uocf::InvalidStreamException&)
        {
            g_UOStatics[i] = nullptr;
            appendToLog("Can't open statics" + std::to_string(i) + ".mul.");
            continue;
        }
    }

    appendToLog("Client Profile \"" + g_clientProfiles[g_loadedClientProfile].m_name + "\" loaded.");
}

