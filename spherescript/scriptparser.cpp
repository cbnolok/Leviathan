#include "scriptparser.h"
#include "../globals.h"
#include "scriptobjects.h"
#include "scriptutils.h"
#include "common.h"
#include <limits>           // for UINT16_MAX macro
#include <QCoreApplication> // for QCoreApplication::processEvents();

#include <QDebug>

ScriptParser::ScriptParser(int profileIndex) :
    m_profileIndex(profileIndex), m_scriptLine(0)
{
    // Parse the scripts and store the data in the ScriptObjTree classes.

    delete g_scriptObjTree_Chars;
    delete g_scriptObjTree_Spawns;
    delete g_scriptObjTree_Items;
    delete g_scriptObjTree_Templates;
    delete g_scriptObjTree_Defs;
    delete g_scriptObjTree_Areas;
    delete g_scriptObjTree_Spells;
    delete g_scriptObjTree_Multis;
    g_scriptObjTree_Chars = new ScriptObjTree();
    g_scriptObjTree_Spawns = new ScriptObjTree();
    g_scriptObjTree_Items = new ScriptObjTree();
    g_scriptObjTree_Templates = new ScriptObjTree();
    g_scriptObjTree_Defs = new ScriptObjTree();
    g_scriptObjTree_Areas = new ScriptObjTree();
    g_scriptObjTree_Spells = new ScriptObjTree();
    g_scriptObjTree_Multis = new ScriptObjTree();
}

/*
ScriptParser::~ScriptParser()
{
}
*/

void ScriptParser::run()
{
    g_loadedScriptsProfile = m_profileIndex;
    // TODO: fai leggere i files da spheretables
    std::vector<std::string> scriptFileList;
    getFilesInDirectorySub(&scriptFileList, g_scriptsProfiles[m_profileIndex].m_scriptsPath);

    appendToLog(std::string("Loading Scripts Profile \"" + g_scriptsProfiles[m_profileIndex].m_name + "\"..."));
    int filesNumber = (int)scriptFileList.size();
    emit notifyPPProgressMax(filesNumber);
    QString msg("Parsing ");
    for (int i = 0; i < filesNumber; i++)
    {
        emit notifyPPMessage(msg + scriptFileList[i].c_str());
        loadFile(scriptFileList[i], false);
        emit notifyPPProgressVal(i);
        QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
    }

    appendToLog("Organizing dupe items...");
    emit notifyPPMessage("Organizing dupe items...");
    emit notifyPPProgressMax(500);
    // Find Dupe items and set them the same Name, Category and Subsection as the Original item.
    size_t dupeObjs_num = m_scriptsDupeItems.size();
    int progressVal = 0;
    for (size_t dupeObj_i = 0; dupeObj_i < dupeObjs_num; dupeObj_i++)
    {
        ScriptObj * dupeObj = m_scriptsDupeItems[dupeObj_i];
        if (dupeObj->m_dupeItem.empty())
            continue;

        bool found = false;
        int dupeNum = ScriptUtils::strToSphereInt(dupeObj->m_dupeItem);
        if (dupeNum != -1)
        {
            // DUPEITEM property is numerical, so an ID
            for (size_t originalCategory_i = 0; originalCategory_i < g_scriptObjTree_Items->m_categories.size(); originalCategory_i++)
            {
                ScriptCategory * originalCategory = g_scriptObjTree_Items->m_categories[originalCategory_i];
                for (size_t originalSubsection_i = 0; originalSubsection_i < originalCategory->m_subsections.size(); originalSubsection_i++)
                {
                    ScriptSubsection * originalSubsection = originalCategory->m_subsections[originalSubsection_i];
                    for (size_t originalObj_i = 0; originalObj_i < originalSubsection->m_objects.size(); originalObj_i++)
                    {
                        ScriptObj * originalObj = originalSubsection->m_objects[originalObj_i];
                        //if (!originalObj->m_dupeItem.empty())   // if it's a dupe item
                        //    continue;
                        if (originalObj->m_ID != dupeNum)
                            continue;
                        found = true;
                        dupeObj->m_description = originalObj->m_description + " - (dupe)";
                        dupeObj->m_category = originalObj->m_category;
                        dupeObj->m_subsection = originalObj->m_subsection;
                        dupeObj->m_subsection->m_objects.push_back(dupeObj);
                        break;
                    }
                    if (found)
                        break;
                }
                if (found)
                    break;
            }
            //if (!found)
            //qDebug() << "NOT FOUND x: " << dupeObj->m_ID << " - " << dupeObj->m_defname.c_str() << " - " << dupeObj->m_dupeItem.c_str() << " - " << dupeNum;
        }
        else
        {
            // DUPEITEM property is a string, so a defname
            for (size_t originalCategory_i = 0; originalCategory_i < g_scriptObjTree_Items->m_categories.size(); originalCategory_i++)
            {
                ScriptCategory * originalCategory = g_scriptObjTree_Items->m_categories[originalCategory_i];
                for (size_t originalSubsection_i = 0; originalSubsection_i < originalCategory->m_subsections.size(); originalSubsection_i++)
                {
                    ScriptSubsection * originalSubsection = originalCategory->m_subsections[originalSubsection_i];
                    for (size_t originalObj_i = 0; originalObj_i < originalSubsection->m_objects.size(); originalObj_i++)
                    {
                        ScriptObj * originalObj = originalSubsection->m_objects[originalObj_i];
                        if (!originalObj->m_dupeItem.empty())   // if it's a dupe item
                            continue;
                        else if (originalObj->m_defname.compare(dupeObj->m_dupeItem) != 0)
                            continue;
                        found = true;
                        dupeObj->m_description = originalObj->m_description + " - (dupe)";
                        dupeObj->m_category = originalObj->m_category;
                        dupeObj->m_subsection = originalObj->m_subsection;
                        dupeObj->m_subsection->m_objects.push_back(dupeObj);
                        break;
                    }
                    if (found)
                        break;
                }
                if (found)
                    break;
            }
            //if (!found)
            //qDebug() << "NOT FOUND y: " << dupeObj->m_ID << " - " << dupeObj->m_defname.c_str() << " - " << dupeObj->m_dupeItem.c_str();
        }

        int progressValNow = (dupeObj_i*500)/dupeObjs_num;
        if (progressValNow > progressVal)
        {
            progressVal = progressValNow;
            emit notifyPPProgressVal(progressVal);
            QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
        }
    }

    appendToLog(std::string("Scripts Profile \"" + g_scriptsProfiles[m_profileIndex].m_name + "\" loaded."));
}


bool ScriptParser::loadFile(std::string filePath, bool loadingResources)
{
    //if (g_scriptObjTree == nullptr)
    //    return false;       // if it wasn't initialized we can't store the objects that will be parsed.

    for (std::vector<std::string>::iterator it = m_loadedScripts.begin(); it != m_loadedScripts.end(); it++)
    {
        if (!filePath.compare(*it))
            return false;   // this file was already loaded.
    }

    std::ifstream fileStream;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fileStream.open(filePath.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open())
    {
        appendToLog(std::string("Error opening file " + filePath));
        return false;
    }
    m_loadedScripts.push_back(filePath);

    appendToLog(std::string("Loading file " + filePath));
    m_scriptLine = 0;

    try
    {
        while ( !fileStream.eof() )
        {         
            std::string line;
            std::getline(fileStream, line);
            if ( fileStream.bad() )
                break;
            m_scriptLine++;

            if ( line.find('[') != std::string::npos )
            {
                //-- Get the pure block string (removing also leading/trailing spaces, trailing \n, \t, \r, \v, \f...)
                size_t index_startBlock = line.find_first_of('[');                
                size_t index_endBlock = line.find_first_of(']');
                if (index_endBlock == std::string::npos)
                    continue;
                size_t index_comment = line.find("//");     // Checking if the block is commented.
                if (index_comment != std::string::npos)
                {
                    if (index_comment < index_startBlock)
                        continue;
                }
                std::string blockStr = line.substr(index_startBlock, index_endBlock-index_startBlock+1);

                //-- Get the keyword
                // Skip eventual spaces and the '[' character before the keyword
                size_t index_keywordLeft = blockStr.find_first_not_of(" \r", 1); // starting from 1 skips the '['
                if (index_keywordLeft == std::string::npos)
                    continue;

                // Skip eventual spaces and the ']' character after the keyword
                size_t index_keywordRight = blockStr.find_first_of("] \r", index_keywordLeft);
                if (index_keywordRight == std::string::npos)
                    continue;

                // Get the pure keyword (it can also be COMMENT, which isn't in the table, so we won't do anything if we encounter it).
                std::string keywordStr = blockStr.substr(index_keywordLeft, index_keywordRight-index_keywordLeft);

                //-- Get the eventual keyword argument (e.g.: [DEFNAME *c_foo*]). We can also have no argument.
                std::string argumentStr("<No Argument>");
                size_t index_argumentLeft = index_keywordRight + 1;
                while ( (blockStr[index_argumentLeft]==' ') || (blockStr[index_argumentLeft]=='\r') )
                {
                    index_argumentLeft++;
                }
                if (blockStr[index_argumentLeft]!=']')  // encountered the end of the block: argument not found
                {
                    size_t index_argumentRight = blockStr.find_first_of("] \r", index_argumentLeft);
                    if (index_argumentRight == std::string::npos)
                        continue;
                    argumentStr = blockStr.substr(index_argumentLeft, index_argumentRight-index_argumentLeft);
                }

                // Look up in the table the ID (enum) of the keyword (resource)
                switch (ScriptUtils::findTableSorted(keywordStr.c_str(), ScriptUtils::resourceBlocks, ScriptUtils::SCRIPTOBJ_RES_QTY))
                {
                case -1:
                    // keyword not found
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_ITEMDEF:
                {
                    ScriptObj *objItem = new ScriptObj();
                    objItem->m_type = SCRIPTOBJ_TYPE_ITEM;
                    objItem->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                    objItem->m_scriptFile = filePath;
                    objItem->m_scriptLine = m_scriptLine;
                    parseBlock(fileStream, objItem);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_MULTIDEF:
                {
                    ScriptObj *objMulti = new ScriptObj();
                    objMulti->m_type = SCRIPTOBJ_TYPE_MULTI;
                    objMulti->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                    objMulti->m_scriptFile = filePath;
                    objMulti->m_scriptLine = m_scriptLine;
                    parseBlock(fileStream, objMulti);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_TEMPLATE:
                {
                    ScriptObj *objTemplate = new ScriptObj();
                    objTemplate->m_type = SCRIPTOBJ_TYPE_TEMPLATE;
                    objTemplate->m_defname = argumentStr;   // using this only as a temporary storage for the argument
                    objTemplate->m_ID = 01;                 // overwrites further IDs findings
                    objTemplate->m_scriptFile = filePath;
                    objTemplate->m_scriptLine = m_scriptLine;
                    parseBlock(fileStream, objTemplate);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_CHARDEF:
                {
                    ScriptObj *objNPC = new ScriptObj();
                    objNPC->m_type = SCRIPTOBJ_TYPE_CHAR;
                    objNPC->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                    objNPC->m_scriptFile = filePath;
                    objNPC->m_scriptLine = m_scriptLine;
                    parseBlock(fileStream, objNPC);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_SPAWN:
                {
                    ScriptObj *objSpawn = new ScriptObj();
                    objSpawn->m_type = SCRIPTOBJ_TYPE_SPAWN;
                    objSpawn->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                    objSpawn->m_scriptFile = filePath;
                    objSpawn->m_scriptLine = m_scriptLine;
                    parseBlock(fileStream, objSpawn);
                }
                    break;
                /*
                case ScriptUtils::SCRIPTOBJ_RES_AREA:
                case ScriptUtils::SCRIPTOBJ_RES_AREADEF:
                case ScriptUtils::SCRIPTOBJ_RES_ROOM:
                case ScriptUtils::SCRIPTOBJ_RES_ROOMDEF:
                {
                    CSObject * pArea = new CSObject;
                    pArea->m_bType = SCRIPTOBJ_TYPE_AREA;
                    pArea->m_csValue = csIndex;
                    pArea->m_csFilename = csFile;

                    csLine = pArea->ReadBlock(*pFile);

                    int iOld = m_aAreas.Find(pArea->m_csValue);
                    if ( iOld != -1 )
                    {
                        CSObject * pOld = (CSObject *) m_aAreas.GetAt(iOld);
                        m_aAreas.RemoveAt(iOld);
                        delete pOld;
                    }
                    m_aAreas.Insert(pArea);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_DEFNAME:
                case ScriptUtils::SCRIPTOBJ_RES_SPHERE:
                case ScriptUtils::SCRIPTOBJ_RES_DEFMESSAGE:
                {
                    while ( bStatus )
                    {
                        bStatus = pFile->ReadString(csLine);
                        if ( !bStatus )
                            break;
                        if ( csLine.Find("[") == 0 )
                        {
                            break;
                        }
                        csLine = csLine.SpanExcluding("//");
                        csLine.Trim();
                        if ( csLine != "" )
                        {
                            //load brain section separately
                            if (csIndex.CompareNoCase("brains") == 0)
                            {
                                CString csBrain, csValue, csTemp;
                                csTemp = csLine.SpanExcluding(" \t=");
                                csValue = csLine.Mid(csLine.FindOneOf(" \t="));
                                csValue.Trim();
                                if ((csTemp != "") && (csValue != ""))
                                {
                                    csBrain.Format("%s (%s)",csTemp,csValue);
                                    m_asaNPCBrains.Insert(csBrain);
                                }
                                continue;
                            }
                            CSObject * pDef = new CSObject;
                            pDef->m_bType = SCRIPTOBJ_TYPE_DEF;
                            CString csTemp;
                            csTemp = csLine.SpanExcluding(" \t=");
                            pDef->m_csValue = csTemp;

                            if ( pDef->m_csValue.GetLength() == csLine.GetLength() )
                            {
                                delete pDef;
                                continue;
                            }

                            int restricted = FindTable(pDef->m_csValue, pRestricted, 4);
                            if (restricted != -1)
                            {
                                delete pDef;
                                continue;
                            }

                            csTemp = csLine.Mid(pDef->m_csValue.GetLength() + 1);
                            csTemp.Trim();
                            if( csTemp.FindOneOf(" \t="))
                                csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
                            csTemp.Trim();

                            if(csTemp.Find('{') != -1)
                            {
                                csTemp = csTemp.Mid(csTemp.Find("{")+1);
                                csTemp = csTemp.Left(csTemp.ReverseFind('}'));
                                if(csTemp.Find('{') != -1)
                                {
                                    csTemp = csTemp.Mid(csTemp.ReverseFind('{')+1);
                                    csTemp = csTemp.SpanExcluding("}");
                                }
                            }

                            pDef->m_csID = csTemp;

                            if ( pDef->m_csValue == "" )
                            {
                                delete pDef;
                                continue;
                            }

                            if ( pDef->m_csValue == pDef->m_csID )
                            {
                                delete pDef;
                                continue;
                            }

                            int iOld = m_aDefList.Find(pDef->m_csValue);
                            if ( iOld != -1 )
                            {
                                CSObject * pOld = (CSObject *) m_aDefList.GetAt(iOld);
                                m_aDefList.RemoveAt(iOld);
                                delete pOld;
                            }
                            m_aDefList.Insert(pDef);
                        }
                    }
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_TYPEDEFS:
                {
                    while ( bStatus )
                    {
                        bStatus = pFile->ReadString(csLine);
                        if ( !bStatus )
                            break;
                        if ( csLine.Find("[") == 0 )
                        {
                            break;
                        }
                        csLine = csLine.SpanExcluding("//");
                        csLine.Trim();
                        if ( csLine != "" )
                        {
                            CString csType, csValue;
                            csType = csLine.SpanExcluding(" \t=");
                            int iOld = m_asaITEMTypes.Find(csType);
                            if ( iOld != -1 )
                                m_asaITEMTypes.RemoveAt(iOld);
                            m_asaITEMTypes.Insert(csType);
                        }
                    }
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_TYPEDEF:
                {
                    int iOld = m_asaITEMTypes.Find(csIndex);
                    if ( iOld != -1 )
                        m_asaITEMTypes.RemoveAt(iOld);
                    m_asaITEMTypes.Insert(csIndex);
                    bStatus = pFile->ReadString(csLine);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_EVENTS:
                {
                    int iOld = m_asaEvents.Find(csIndex);
                    if ( iOld != -1 )
                        m_asaEvents.RemoveAt(iOld);
                    m_asaEvents.Insert(csIndex);
                    bStatus = pFile->ReadString(csLine);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_FUNCTION:
                {
                    int iOld = m_asaFunctions.Find(csIndex);
                    if ( iOld != -1 )
                        m_asaFunctions.RemoveAt(iOld);
                    m_asaFunctions.Insert(csIndex);
                    bStatus = pFile->ReadString(csLine);
                }
                    break;
                /*  // TO-DO
                case SCRIPTOBJ_RES_LISTS:
                    break
                */
                /*
                case ScriptUtils::SCRIPTOBJ_RES_SPELL:
                {
                    CSObject * pSpell = new CSObject;
                    pSpell->m_bType = SCRIPTOBJ_TYPE_SPELL;
                    pSpell->m_csValue = csIndex;
                    pSpell->m_csFilename = csFile;

                    csLine = pSpell->ReadBlock(*pFile);

                    int iOld = m_aSpellList.Find(pSpell->m_csValue);
                    if ( iOld != -1 )
                    {
                        CSObject * pOld = (CSObject *) m_aSpellList.GetAt(iOld);
                        m_aSpellList.RemoveAt(iOld);
                        delete pOld;
                    }
                    m_aSpellList.Insert(pSpell);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_SKILL:
                {
                    while ( bStatus )
                    {
                        bStatus = pFile->ReadString(csLine);
                        if ( !bStatus )
                            break;
                        if ( csLine.Find("[") == 0 )
                        {
                            break;
                        }
                        csLine = csLine.SpanExcluding("//");
                        csLine.Trim();
                        if ( csLine != "" )
                        {
                            CString csKey, csTemp;
                            csTemp = csLine.SpanExcluding(" \t=");

                            if ( csTemp.CompareNoCase("Key") != 0 )
                                continue;

                            csTemp = csLine.Mid(csTemp.GetLength() + 1);
                            csTemp.Trim();
                            if( csTemp.FindOneOf(" \t="))
                                csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
                            csTemp.Trim();
                            csKey.Format("%s (%s)", csTemp, csIndex);
                            if (csKey != "")
                                m_asaNPCSkills.Insert(csKey);
                        }
                    }
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_WORLDITEM:
                case ScriptUtils::SCRIPTOBJ_RES_WI:
                {
                    if (csIndex.CompareNoCase("i_worldgem_bit") == 0)
                    {
                        int iType = ITEM_SPAWN_CHAR;
                        CString csPos;
                        while ( bStatus )
                        {
                            bStatus = pFile->ReadString(csLine);
                            if ( !bStatus )
                                break;
                            if ( csLine.Find("[") == 0 )
                            {
                                break;
                            }
                            csLine = csLine.SpanExcluding("//");
                            csLine.Trim();
                            if ( csLine != "" )
                            {
                                CString csTemp;
                                csTemp = csLine.SpanExcluding(" \t=");

                                if ( csTemp.CompareNoCase("TYPE") == 0 )
                                {
                                    csTemp = csLine.Mid(csTemp.GetLength() + 1);
                                    csTemp.Trim();
                                    if( csTemp.FindOneOf(" \t="))
                                        csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
                                    csTemp.Trim();
                                    if ( csTemp.CompareNoCase("t_spawn_item") == 0 )
                                        iType = ITEM_SPAWN_ITEM;
                                    else
                                        iType = 0;
                                }

                                if ( csTemp.CompareNoCase("P") == 0 )
                                {
                                    csPos = csLine.Mid(csTemp.GetLength() + 1);
                                    csPos.Trim();
                                    if( csPos.FindOneOf(" \t="))
                                    {
                                        csPos = csPos.Mid(csPos.FindOneOf(" \t=") + 1);
                                        csPos.Trim();
                                    }
                                }
                            }
                        }
                        if (iType == ITEM_SPAWN_ITEM)
                            m_asaSPAWNitem.Insert(csPos);
                        else if (iType == ITEM_SPAWN_CHAR)
                            m_asaSPAWNchar.Insert(csPos);
                    }
                    else
                        bStatus = pFile->ReadString(csLine);
                }
                    break;
                case ScriptUtils::SCRIPTOBJ_RES_RESOURCES:
                {
                    if(bResource)
                    {
                        while ( bStatus )
                        {
                            bStatus = pFile->ReadString(csLine);
                            if ( !bStatus )
                                break;
                            if ( csLine.Find("[") == 0 )
                            {
                                break;
                            }
                            csLine = csLine.SpanExcluding("//");
                            csLine.Trim();
                            if ( csLine != "" )
                            {
                                CString csLoadFile;
                                csLoadFile.Format("%s\\%s",csFile.Left(csFile.ReverseFind('\\')),csLine.Mid(csLine.FindOneOf("/\\")+1));
                                csLoadFile.Replace('/', '\\');
                                if( csLine.Right(4).CompareNoCase(".scp") == 0)
                                {
                                    CStdioFile * pLoadFile = new CStdioFile;
                                    if ( !pLoadFile->Open(csLoadFile, CFile::modeRead | CFile::shareDenyNone) )
                                    {
                                        Main->m_log.Add(1,"ERROR: Unable to open file %s", csLoadFile);
                                        continue;
                                    }
                                    LoadFile(pLoadFile);
                                    pLoadFile->Close();
                                }
                                else
                                {
                                    CString csLoadDir = csLoadFile.Left(csLoadFile.ReverseFind('\\'));
                                    LoadSingleDirectory(csLoadDir);
                                }
                            }
                        }
                    }
                    bStatus = pFile->ReadString(csLine);
                }
                    break;
                */
                //Ignore these blocks
                case ScriptUtils::SCRIPTOBJ_RES_COMMENT:
                case ScriptUtils::SCRIPTOBJ_RES_UNKNOWN:
                case ScriptUtils::SCRIPTOBJ_RES_ADVANCE:
                case ScriptUtils::SCRIPTOBJ_RES_BLOCKIP:
                case ScriptUtils::SCRIPTOBJ_RES_BOOK:
                case ScriptUtils::SCRIPTOBJ_RES_DIALOG:
                case ScriptUtils::SCRIPTOBJ_RES_FAME:
                case ScriptUtils::SCRIPTOBJ_RES_GLOBALS:
                case ScriptUtils::SCRIPTOBJ_RES_GMPAGE:
                case ScriptUtils::SCRIPTOBJ_RES_KARMA:
                case ScriptUtils::SCRIPTOBJ_RES_MENU:
                case ScriptUtils::SCRIPTOBJ_RES_MOONGATES:
                case ScriptUtils::SCRIPTOBJ_RES_NAMES:
                case ScriptUtils::SCRIPTOBJ_RES_NEWBIE:
                case ScriptUtils::SCRIPTOBJ_RES_NOTOTITLES:
                case ScriptUtils::SCRIPTOBJ_RES_OBSCENE:
                case ScriptUtils::SCRIPTOBJ_RES_PLEVEL:
                case ScriptUtils::SCRIPTOBJ_RES_REGIONRESOURCE:
                case ScriptUtils::SCRIPTOBJ_RES_REGIONTYPE:
                case ScriptUtils::SCRIPTOBJ_RES_RUNES:
                case ScriptUtils::SCRIPTOBJ_RES_SECTOR:
                case ScriptUtils::SCRIPTOBJ_RES_SERVERS:
                case ScriptUtils::SCRIPTOBJ_RES_SKILLCLASS:
                case ScriptUtils::SCRIPTOBJ_RES_SKILLMENU:
                case ScriptUtils::SCRIPTOBJ_RES_SPEECH:
                case ScriptUtils::SCRIPTOBJ_RES_STARTS:
                case ScriptUtils::SCRIPTOBJ_RES_TELEPORTERS:
                case ScriptUtils::SCRIPTOBJ_RES_TIMERF:
                case ScriptUtils::SCRIPTOBJ_RES_WEBPAGE:
                case ScriptUtils::SCRIPTOBJ_RES_WORLDCHAR:
                case ScriptUtils::SCRIPTOBJ_RES_WC:
                case ScriptUtils::SCRIPTOBJ_RES_WS:
                case ScriptUtils::SCRIPTOBJ_RES_QTY:
                    break;
                default:
                    //appendToLog("Unknown keyword \"" + keywordStr + "\" in file " + filePath + ".");
                    break;
                }
            }
        }
    }
    catch (std::ios_base::failure& e)
    {
        appendToLog("ERROR: Caught an exception while reading the file " + filePath + ". Error code: " + std::to_string(e.code().value()) + ". Message: " + e.what() + ".");
    }
    return true;
}

void ScriptParser::parseBlock(std::ifstream &fileStream, ScriptObj *obj)
{
    bool ignoreTrigger = false;

    std::string objSubsection(SCRIPTSUBSECTION_NONE_NAME);   // Put it in a temporary string, since the ScriptCategory class may not be
                                                             //  instantiated when we read the Subsection value.
    std::string objDescription;     // If the value is '@', then the Description should have the same value than the Name.
    std::string objName;
    std::string objDefname;         // It can be the in the block's header or with the DEFNAME keyword, we'll sort it out later.
    int objID = -1;                 // Same as for the DEFNAME.

    std::string objArgument = obj->m_defname;
    obj->m_defname.clear();


    while ( !fileStream.eof() )
    {
        std::string line;
        std::streampos pos = fileStream.tellg();
        std::getline(fileStream, line);
        if ( fileStream.bad() )
            break;

        //appendToLog(std::string("Reading line " + line));
        m_scriptLine++;

        // Check if we are in a new block.
        if ( line.find('[') != std::string::npos )
        {
            fileStream.seekg(pos);
            break;
        }

        //-- Check if it's a trigger and if we have to parse the values inside it.
        // There can be spaces between '=' and '@', or even "ON @Create" is legit, so we have to recognize all of them.
        //  Removing whitespaces and '=' symbols, we should have only "ON@Create"
        // Also, valid assignations are both "DEFNAME= foo" and "DEFNAME  foo".
        std::string tempLine(line);
        strToUpper(tempLine);       // need to put all to uppercase since std::string::find is case-sensitive.
        size_t i_temp;
        while ( (i_temp = tempLine.find_first_of(" \r=")) != std::string::npos )
        {
            tempLine.erase(i_temp, 1);
        }
        unsigned int prefixPos = tempLine.find("ON@");
        if (prefixPos != std::string::npos)
        {
            // We care only about keywords under the header or inside the @Create trigger.
            if (tempLine.find("DESCRIPTION@") == std::string::npos)    // "DESCRIPTION@" contains "ON@"! If it's not the case, check the following...
            {
                if (tempLine.find("CREATE", prefixPos + 3) != std::string::npos)
                    ignoreTrigger = false;
                else
                    ignoreTrigger = true;
                continue;   // No need to further examine this string, since it's the head of a trigger.
            }
        }

        //-- The line is not the trigger head, so separate the keyword from the value.
        // Remove leading spaces or tabulation escapes (e.g.: " \t DEFNAME c_foo").
        unsigned int keywordStart = 0, keywordEnd;
        for (; keywordStart < line.length(); keywordStart++)
        {
            if (!isspace(line[keywordStart]))
                break;
        }

        // Get the position of the Keyword.
        //  We can have both spaces before the '=' symbol or directly the value after some spaces.
        unsigned int delimiterIndex;
        delimiterIndex = line.find_first_of('=', keywordStart);
        if (delimiterIndex != std::string::npos)    // If there's a '=' symbol.
        {
            keywordEnd = line.find_first_of(" \r", keywordStart);
            if ( (keywordEnd == std::string::npos) || (keywordEnd > delimiterIndex ) )  // No spaces found after the keyword.
                keywordEnd = delimiterIndex;
        }
        else        // If there's not a '=' symbol.
        {
            delimiterIndex = line.find_first_of(" \r", keywordStart);
            if (delimiterIndex != std::string::npos)
            {
                // Go to the first non-whitespace character
                keywordEnd = delimiterIndex;
            }
            else
            {
                continue;   // invalid?
            }
        }

        // Get the position of the Value.
        unsigned int valueStart, valueEnd;
        //  Skip eventual whitespaces before the Value
        for (valueStart = keywordEnd+1; valueStart < line.length(); valueStart++)
        {
            if (!isspace(line[valueStart+1]))
                break;
        }
        //  Get the position of the last character of the Value.
        valueEnd = line.find_first_of("\r", valueStart);
        if (valueEnd == std::string::npos)
            valueEnd = line.find_first_of("\t\n\v\f", valueStart);
        while (valueEnd > valueStart && line[valueEnd] == ' ')
            --valueEnd;

        // Finally separate the keyword from the value.
        std::string keyword = line.substr(keywordStart, keywordEnd - keywordStart);
        std::string value = line.substr(valueStart, valueEnd - valueStart);
        //appendToLog(std::string("Keyword:" + keyword + " - Value:" + value));

        switch (ScriptUtils::findTableSorted(keyword.c_str(), ScriptUtils::objectTags, ScriptUtils::TAG_QTY))
        {
        case ScriptUtils::TAG_CATEGORY:
            obj->m_category = objTree(obj->m_type)->findCategory(value);
            break;
        case ScriptUtils::TAG_SUBSECTION:
            objSubsection = value;
            break;
        case ScriptUtils::TAG_DESCRIPTION:
            objDescription = value;
            break;
        case ScriptUtils::TAG_DUPEITEM:
            obj->m_dupeItem = value;

            // Put temporarily the dupe items in a special Dupe Items Subsection. When parsing is completed,
            // overwrite Category and Subsection for dupe items: they will have the same as the main/original item.
            // Also the name will be inherited.
            m_scriptsDupeItems.push_back(obj);
            goto GOTO_DUPEITEM;
            break;
        case ScriptUtils::TAG_DEFNAME:
            objDefname = value;
            break;
        case ScriptUtils::TAG_ID:
        {
            if (ignoreTrigger || obj->m_ID)
                break;
            objID = ScriptUtils::strToSphereInt(value);
        }
            break;
        case ScriptUtils::TAG_COLOR:
        {
            if (ignoreTrigger)
                break;
            // The color is a 16 bits number.
            int tempColor = ScriptUtils::strToSphereInt(value);
            obj->m_color = (tempColor > UINT16_MAX) ? 0 : (unsigned short)tempColor;
            break;
        }
        case ScriptUtils::TAG_NAME:
        {
            if (ignoreTrigger)
                break;
            objName = value;
        }
            break;
        /*
        case ScriptUtils::TAG_GROUP:
        {
            if (bIgnore)
                break;
            if (this->m_bType == SCRIPTOBJ_TYPE_AREA)
                this->m_csSubsection = csValue;
            break;
        }
        case ScriptUtils::TAG_P:
        {
            if (bIgnore)
                break;
            if (this->m_bType == SCRIPTOBJ_TYPE_AREA)
            {
                CString csX, csY, csZ, csM;
                csX = csValue.SpanExcluding(",");
                csY = csValue.Mid(csValue.Find(",") + 1);
                if (csY.Find(",") != -1)
                    csZ = csY.Mid(csY.Find(",") + 1);
                else
                    csZ = "0";
                csY = csY.SpanExcluding(",");
                if (csZ.Find(",") != -1)
                    csM = csZ.Mid(csZ.Find(",") + 1);
                else
                    csM = "0";
                csZ = csZ.SpanExcluding(",");
                this->m_csID = CFMsg("%1,%2,%3", csX, csY, csZ);
                this->m_csDisplay = csM;
                this->m_csCategory = CFMsg(CMsg("IDS_MAP_REGION"), atoi(csM));
            }
            break;
        }
        case ScriptUtils::TAG_POINT:
        {
            if (bIgnore)
                break;
            if (this->m_bType == SCRIPTOBJ_TYPE_AREA)
            {
                CString csX, csY, csZ, csM, csTemp;
                csX = csValue.SpanExcluding(",");
                csY = csValue.Mid(csValue.Find(",") + 1);
                if (csY.Find(",") != -1)
                    csZ = csY.Mid(csY.Find(",") + 1);
                else
                    csZ = "0";
                csY = csY.SpanExcluding(",");
                if (csZ.Find(",") != -1)
                    csM = csZ.Mid(csZ.Find(",") + 1);
                else
                    csM = "0";
                csZ = csZ.SpanExcluding(",");
                csTemp.Format("%s,%s,%s", csX, csY, csZ);
                this->m_csID = csTemp;
                this->m_csDisplay = csM;
                break;
            }
        }
        */
        }   // this closes the switch clause
    }   // this closes the while loop


    if (obj->m_category != nullptr)
    {
        obj->m_subsection = obj->m_category->findSubsection(objSubsection);
        obj->m_subsection->m_objects.push_back(obj);    // Add the newly populated object to the subsection object vector.
    }
    else
    {
        obj->m_category = objTree(obj->m_type)->findCategory(SCRIPTCATEGORY_NONE_NAME);
        obj->m_subsection = obj->m_category->findSubsection(objSubsection);
        obj->m_subsection->m_objects.push_back(obj);
    }

    // Use NAME (or Subsection, if there isn't a name) instead of DESCRIPTION
    //  if DESCRIPTION == '@' or if there isn't a DESCRIPTION.
    if (objDescription.empty())
    {
        if (!objName.empty())
            obj->m_description = objName;
        else if (obj->m_subsection->m_subsectionName.compare(SCRIPTSUBSECTION_NONE_NAME) != 0)
            obj->m_description = obj->m_subsection->m_subsectionName;
    }
    else
    {
        if ( (objDescription.length() == 1) && (objDescription[0] == '@') )
        {
            if (!objName.empty())
                obj->m_description = objName;
            else if (obj->m_subsection->m_subsectionName.compare(SCRIPTSUBSECTION_NONE_NAME) != 0)
                obj->m_description = obj->m_subsection->m_subsectionName;
        }
        else
            obj->m_description = objDescription;
    }

    // If it's a Dupe Item, skip Category, Subsection and Name assignation: they will be inherited by the Original Item later
    //  (also most probably they aren't specified in the script).
    GOTO_DUPEITEM:

    // Detect if the block header argument is the DEFNAME or the ID.
    int objIDHeader =  ScriptUtils::strToSphereInt(objArgument);
    if (objIDHeader == -1)  // It's a DEFNAME.
    {
        if (!objArgument.empty())
            obj->m_defname = objArgument;

        if (objID != -1)
            obj->m_ID = objID;
    }
    else                    // It's an ID.
    {
        if (!objDefname.empty())
            obj->m_defname = objDefname;

        if (objID != -1)    // There's an "override" for the ID.
            obj->m_ID = objID;
        else
            obj->m_ID = objIDHeader;

    }
}

