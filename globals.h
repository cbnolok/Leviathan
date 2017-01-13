#ifndef GLOBALS_H
#define GLOBALS_H

#include "spherescript/scriptsprofile.h"
#include "spherescript/scriptobjects.h"
#include "forms/maintab_log.h"


extern int g_loadedScriptsProfile;                    // index of the current scripts profile
extern std::vector<ScriptsProfile> g_scriptsProfiles;  // contains all of the scripts profiles

// containers for Sphere Objects: characters, items, maps...
extern ScriptObjTree *g_scriptObjTree_Chars;
extern ScriptObjTree *g_scriptObjTree_Spawns;
extern ScriptObjTree *g_scriptObjTree_Items;
extern ScriptObjTree *g_scriptObjTree_Templates;
extern ScriptObjTree *g_scriptObjTree_Defs;
extern ScriptObjTree *g_scriptObjTree_Areas;
extern ScriptObjTree *g_scriptObjTree_Spells;
extern ScriptObjTree *g_scriptObjTree_Multis;

ScriptObjTree * objTree(int objType);           // returns the right global object tree for the given script item type

extern MainTab_Log *g_MainTab_Log_inst;
bool appendToLog(const char* str);
bool appendToLog(const std::string &str);


#endif // GLOBALS_H
