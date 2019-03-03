#ifndef GLOBALS_H
#define GLOBALS_H

#include <functional>
#include <vector>
#include "settings/appsettings.h"
#include "settings/clientprofile.h"
#include "settings/scriptsprofile.h"


extern AppSettings g_settings;     // contains general settings

// Client/Scripts profiles
extern int g_loadedClientProfile;                       // index of the current client profile
extern std::vector<ClientProfile> g_clientProfiles;     // contains all of the client profiles

extern int g_loadedScriptsProfile;                      // index of the current scripts profile
extern std::vector<ScriptsProfile> g_scriptsProfiles;   // contains all of the scripts profiles
extern std::vector<std::string> g_scriptFileList;       // contains the absolute path of each script of the profile loaded

extern bool g_sendKeystrokeAndFocusClient;              // for KeystrokeSender

// containers for Sphere Objects: characters, items, maps...
class ScriptObjTree;
extern ScriptObjTree *g_scriptObjTree_Chars;
extern ScriptObjTree *g_scriptObjTree_Spawns;
extern ScriptObjTree *g_scriptObjTree_Items;
extern ScriptObjTree *g_scriptObjTree_Templates;
extern ScriptObjTree *g_scriptObjTree_Defs;
extern ScriptObjTree *g_scriptObjTree_Areas;
extern ScriptObjTree *g_scriptObjTree_Spells;
extern ScriptObjTree *g_scriptObjTree_Multis;

ScriptObjTree * getScriptObjTree(int objType);           // returns the right global object tree for the given script item type


// Log stuff

void appendToLog(const std::string &str);


// Client files stuff

namespace uocf
{
    class UOArt;
    class UOAnim;
    class UOHues;
    class UOMap;
    class UOStatics;
    class UORadarCol;
}
extern uocf::UOArt  *g_UOArt;
extern uocf::UOAnim *g_UOAnim;
extern uocf::UOHues *g_UOHues;
extern uocf::UORadarCol *g_UORadarCol;
extern std::vector<uocf::UOMap *> g_UOMaps;
extern std::vector<uocf::UOStatics *> g_UOStatics;

void loadClientFiles(std::function<void(int)> reportProgress);


#endif // GLOBALS_H
