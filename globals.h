#ifndef GLOBALS_H
#define GLOBALS_H

#include "settings/clientprofile.h"
#include "settings/scriptsprofile.h"
#include "forms/maintab_log.h"


// Client/Scripts profiles

extern int g_loadedClientProfile;                       // index of the current client profile
extern std::vector<ClientProfile> g_clientProfiles;     // contains all of the client profiles

extern int g_loadedScriptsProfile;                      // index of the current scripts profile
extern std::vector<ScriptsProfile> g_scriptsProfiles;   // contains all of the scripts profiles
extern std::vector<std::string> g_scriptFileList;       // contains the absolute path of each script of the profile loaded

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

ScriptObjTree * objTree(int objType);           // returns the right global object tree for the given script item type


// Log stuff

extern MainTab_Log *g_MainTab_Log_inst;
bool appendToLog(const char* str);
bool appendToLog(const std::string &str);


// Client files stuff

class UOHues;
extern UOHues *g_UOHues;

class UOArt;
extern UOArt *g_UOArt;

class UOAnim;
extern UOAnim *g_UOAnim;

void loadClientFiles();


#endif // GLOBALS_H
