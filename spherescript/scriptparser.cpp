#include "scriptparser.h"
#include "../globals.h"
#include "../common.h"
#include "../sysio.h"
#include "scriptobjects.h"
#include "scriptutils.h"
#include <QCoreApplication> // for QCoreApplication::processEvents();

#include <QDebug>


ScriptParser::ScriptParser(int profileIndex) :
    m_profileIndex(profileIndex), m_scriptLine(0)
{
    // Parse the scripts and store the data in the ScriptObjTree classes.

    g_scriptFileList.clear();

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
    getFilesInDirectorySub(&g_scriptFileList, g_scriptsProfiles[m_profileIndex].m_scriptsPath);


    /*  Store in memory scripts data    */

    appendToLog(std::string("Loading Scripts Profile \"" + g_scriptsProfiles[m_profileIndex].m_name + "\"..."));
    int filesNumber = (int)g_scriptFileList.size();
    emit notifyTPProgressMax(filesNumber);
    //QString msg("Parsing ");
    emit notifyTPMessage("Parsing scripts");
    for (int i = 0; i < filesNumber; i++)
    {
        //emit notifyTPMessage(msg + g_scriptFileList[i].c_str());
        loadFile(i, false);
        emit notifyTPProgressVal(i);
        QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
    }


    /*  Find Dupe items and set them the same Name, Category and Subsection as the Original item    */

    appendToLog("Organizing dupe items...");
    emit notifyTPMessage("Organizing dupe items...");
    emit notifyTPProgressMax(150);

    size_t dupeObjs_num = m_scriptsDupeItems.size();
    int progressVal = 0;
    for (size_t dupeObj_i = 0; dupeObj_i < dupeObjs_num; dupeObj_i++)
    {
        bool found = false;
        ScriptObj * dupeObj = m_scriptsDupeItems[dupeObj_i];
        if (dupeObj->m_dupeItem.empty())
            continue;   // error?

        bool isDUPEITEMnumerical = isStringNumericHex(dupeObj->m_dupeItem);
        for (auto it = m_scriptsDupeParents.begin(), end = m_scriptsDupeParents.end(); it != end; it++)
        {
            ScriptObj * parentObj = *it;
            //if (!parentObj->m_dupeItem.empty())   // if it's a dupe item
            //    continue; // dupe item having as parent another dupe item? error in scripts?
            if (!parentObj->m_baseDef)    // it may be a child item which has ben set ID = base item
                continue;

            if (isDUPEITEMnumerical)    // DUPEITEM property is numerical, so it's a ID
            {
                if (parentObj->m_ID != dupeObj->m_dupeItem)
                    continue;
            }
            else                        // DUPEITEM property is a string, so it's a defname
            {
                if (parentObj->m_defname != dupeObj->m_dupeItem)
                    continue;
            }

            found = true;
            dupeObj->m_description = parentObj->m_description + " - (dupe)";
            dupeObj->m_category = parentObj->m_category;
            dupeObj->m_subsection = parentObj->m_subsection;
            dupeObj->m_subsection->m_objects.push_back(dupeObj);
            break;
        }

        if (!found)
            appendToLog("[WARNING](Dupe) Couldn't find Parent Item (" + dupeObj->m_dupeItem + ") " +
                        "for Dupe Item -> Defname=" + dupeObj->m_defname + ", ID=" + dupeObj->m_ID + ". " +
                        "File: " + g_scriptFileList[dupeObj->m_scriptFileIndex]);

        int progressValNow = (int)( (dupeObj_i*150)/dupeObjs_num );
        if (progressValNow > progressVal)
        {
            progressVal = progressValNow;
            emit notifyTPProgressVal(progressVal);
            QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
        }
    }


    /*  Get the ID of the item/animation to show for the child objects (derived from another item/char) */

    appendToLog("Assigning the displayID to child objects...");
    emit notifyTPMessage("Assigning the displayID to child objects...");
    emit notifyTPProgressMax(150);
    progressVal = 0;

    ScriptObjTree*              trees[]         = { g_scriptObjTree_Items,  g_scriptObjTree_Chars   };
    std::deque<ScriptObj*>*     childObjects[]  = { &m_scriptsChildItems,   &m_scriptsChildChars    };
    size_t childItemsNum = m_scriptsChildItems.size() + m_scriptsChildChars.size();
    size_t childrenProcessed = 0;
    for (int tree_i = 0; tree_i < 2; tree_i++)
    {
        // Iterate one time for the items and one for the chars

        auto curTree            = trees[tree_i];
        auto curChildObjects    = childObjects[tree_i];
        for (size_t child_i = 0, child_s = curChildObjects->size(); child_i < child_s; child_i++)
        {
            // Iterate through each object of the tree.
            // Now look inside each object of each subsection of each category to find the parent object.

            bool found = false;
            ScriptObj* childObj = (*curChildObjects)[child_i];
            bool isChildIDNumeric = isStringNumericHex(childObj->m_ID);

            for (size_t x = 0, xe = curTree->m_categories.size(); x < xe; x++)
            {
                ScriptCategory * originalCategory = curTree->m_categories[x];
                for (size_t y = 0, ye = originalCategory->m_subsections.size(); y < ye; y++)
                {
                    ScriptSubsection * z = originalCategory->m_subsections[y];
                    for (size_t w = 0, we = z->m_objects.size(); w < we; w++)
                    {
                        ScriptObj * parentObj = z->m_objects[w];
                        if (!parentObj->m_baseDef)    // it may be a child object which has ben set ID = base object
                            continue;

                        if (isChildIDNumeric)           // the object has ID = number
                        {
                            if (parentObj->m_ID != childObj->m_ID)
                                continue;
                        }
                        else                            // the object has ID = defname
                        {
                            if (parentObj->m_defname != childObj->m_ID)
                                continue;
                        }

                        found = true;
                        childObj->m_display = parentObj->m_display;
                        break;
                    }
                    if (found)
                        break;
                }
                if (found)
                    break;
            }

            /*
            for (auto it = trees[tree_i]->begin(), end = trees[tree_i]->end(); it != end; ++it)
            {
                ScriptObj * parentObj = *it;
                if (!parentObj->m_baseDef)    // it may be a child object which has ben set ID = base object
                    continue;

                if (isChildIDNumeric)           // the object has ID = number
                {
                    if (parentObj->m_ID != childObj->m_ID)
                        continue;
                }
                else                            // the object has ID = defname
                {
                    if (parentObj->m_defname != childObj->m_ID)
                        continue;
                }

                found = true;
                childObj->m_display = parentObj->m_display;
                break;
            }

            if (!found)
                appendToLog("[WARNING](displayID) Couldn't find Parent Object (" + childObj->m_ID + ") " +
                            "for Child Object -> Defname=" + childObj->m_defname + ", ID=" + childObj->m_ID + ". " +
                            "File: " + g_scriptFileList[childObj->m_scriptFileIndex]);
            */


            childrenProcessed++;
            int progressValNow = (int)( (childrenProcessed*150)/childItemsNum );
            if (progressValNow > progressVal)
            {
                progressVal = progressValNow;
                emit notifyTPProgressVal(progressVal);
                QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
            }

        }   // end of child iterating for loop
    }       // end of the tree iterating for loop


    appendToLog(std::string("Scripts Profile \"" + g_scriptsProfiles[m_profileIndex].m_name + "\" loaded."));
}


bool ScriptParser::loadFile(int fileIndex, bool loadingResources)
{
    //if (g_scriptObjTree == nullptr)
    //    return false;       // if it wasn't initialized we can't store the objects that will be parsed.

    std::string& filePath = g_scriptFileList[fileIndex];

    for (std::string& f : m_loadedScripts)
    {
        if (filePath == f)
            return false;   // this file was already loaded.
    }

    std::ifstream fileStream;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fileStream.open(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open())
    {
        appendToLog(std::string("Error opening file " + filePath));
        return false;
    }
    m_loadedScripts.push_back(filePath);

    appendToLog(std::string("Loading file " + filePath));
    m_scriptLine = 0;

//    try
//    {
        while ( !fileStream.eof() )
        {         
            std::string line;
            std::getline(fileStream, line);
            if ( fileStream.bad() )
                break;
            m_scriptLine++;

            if ( line.find('[') == std::string::npos )
                continue;

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
            switch (ScriptUtils::findTableSorted(keywordStr, ScriptUtils::resourceBlocks, ScriptUtils::SCRIPTOBJ_RES_QTY - 1))
            {
            case -1:
                // keyword not found
                break;
            case ScriptUtils::SCRIPTOBJ_RES_ITEMDEF:
            {
                ScriptObj *objItem = new ScriptObj();
                objItem->m_type = SCRIPTOBJ_TYPE_ITEM;
                objItem->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                objItem->m_scriptFileIndex = fileIndex;
                objItem->m_scriptLine = m_scriptLine;
                parseBlock(fileStream, objItem);
            }
                break;
            case ScriptUtils::SCRIPTOBJ_RES_MULTIDEF:
            {
                ScriptObj *objMulti = new ScriptObj();
                objMulti->m_type = SCRIPTOBJ_TYPE_MULTI;
                objMulti->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                objMulti->m_scriptFileIndex = fileIndex;
                objMulti->m_scriptLine = m_scriptLine;
                objMulti->m_display = 0x22c4;     // mini house
                parseBlock(fileStream, objMulti);
            }
                break;
            case ScriptUtils::SCRIPTOBJ_RES_TEMPLATE:
            {
                ScriptObj *objTemplate = new ScriptObj();
                objTemplate->m_type = SCRIPTOBJ_TYPE_TEMPLATE;
                objTemplate->m_defname = argumentStr;   // using this only as a temporary storage for the argument
                objTemplate->m_ID = "01";               // overwrites further IDs findings
                objTemplate->m_scriptFileIndex = fileIndex;
                objTemplate->m_scriptLine = m_scriptLine;
                objTemplate->m_display = 0xe76;     // bag
                parseBlock(fileStream, objTemplate);
            }
                break;
            case ScriptUtils::SCRIPTOBJ_RES_CHARDEF:
            {
                ScriptObj *objNPC = new ScriptObj();
                objNPC->m_type = SCRIPTOBJ_TYPE_CHAR;
                objNPC->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                objNPC->m_scriptFileIndex = fileIndex;
                objNPC->m_scriptLine = m_scriptLine;
                parseBlock(fileStream, objNPC);
            }
                break;
            case ScriptUtils::SCRIPTOBJ_RES_SPAWN:
            {
                ScriptObj *objSpawn = new ScriptObj();
                objSpawn->m_type = SCRIPTOBJ_TYPE_SPAWN;
                objSpawn->m_defname = argumentStr;        // using this only as a temporary storage for the argument
                objSpawn->m_scriptFileIndex = fileIndex;
                objSpawn->m_scriptLine = m_scriptLine;
                objSpawn->m_display = 0x3a;     // wisp
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
//    }
//    catch (std::ios_base::failure& e)
//    {
//        appendToLog("ERROR: Caught an exception while reading the file " + filePath + ". Error code: " + std::to_string(e.code().value()) + ". Message: " + e.what() + ".");
//    }
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
    std::string objID;              // Same as for the DEFNAME.

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

        // Check if we are in a new block, in this case we have to stop.
        if ( line.find('[') != std::string::npos )
        {
            fileStream.seekg(pos);
            break;
        }

        m_scriptLine++;

        // Remove leading spaces
        size_t linestart = 0;
        while ( isspace(line[linestart]) && linestart < line.length() )
            linestart++;

        // Checking if the block is commented.
        size_t index_comment = line.rfind("//", linestart + 1);     // reverse find, starting from the second character (position 1)
        if (index_comment != std::string::npos)
                continue;

        //-- Check if it's a trigger and if we have to parse the values inside it.
        // There can be spaces between '=' and '@', or even "ON @Create" is legit, so we have to recognize all of them.
        //  Removing whitespaces and '=' symbols, we should have only "ON@Create"
        // Also, valid assignations are both "DEFNAME= foo" and "DEFNAME  foo".
        std::string tempLine(line);
        strToUpper(tempLine);       // need to put all to uppercase since std::string::find is case-sensitive.
        size_t prefixPos = tempLine.find("ON@", linestart);
        if (prefixPos != std::string::npos)
        {
            // We care only about keywords under the header or inside the @Create trigger.
            if (tempLine.find("DESCRIPTION@") == std::string::npos)    // "DESCRIPTION@" contains "ON@"! If it's not the case, check the following...
            {
                if (tempLine.find("CREATE", linestart + prefixPos + 3) != std::string::npos)
                    ignoreTrigger = false;
                else
                    ignoreTrigger = true;
                continue;   // No need to further examine this string, since it's the head of a trigger.
            }
        }

        //-- The line is not the trigger head, so separate the keyword from the value.
        // Remove leading spaces or tabulation escapes (e.g.: " \t DEFNAME c_foo").
        size_t keywordStart, keywordEnd;
        keywordStart = linestart;

        // Get the position of the Keyword.
        //  We can have both spaces before the '=' symbol or directly the value after some spaces.
        size_t delimiterIndex;
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
        size_t valueStart, valueEnd;

        //  Skip eventual whitespaces before the Value
        for (valueStart = keywordEnd + 1; valueStart < line.length(); valueStart++)
        {
            if (!isspace(line[valueStart]))
                break;
        }

        //  Get the position of the last character of the Value.
        valueEnd = line.find("//", valueStart); // skip comments at the end of the line
        if (valueEnd == std::string::npos)
            valueEnd = line.length();
        valueEnd--;
        while ( (valueEnd > valueStart) && (isspace(line[valueEnd]) || line[valueEnd] == '\n') )
            --valueEnd;
        valueEnd++;     // to have the character number (starting from 1), instead of having the position (0-based)

        // Finally separate the keyword from the value.
        std::string keyword = line.substr(keywordStart, keywordEnd - keywordStart);
        std::string value = line.substr(valueStart, valueEnd - valueStart);
        //appendToLog(std::string("Keyword:*" + keyword + "* - Value:*" + value + "*"+ ". line:*" +line +"*"));
        //appendToLog(std::string("keystart:" + std::to_string(keywordStart) + "-end:" + std::to_string(keywordEnd)));
        //appendToLog(std::string("valstart:" + std::to_string(valueStart) + "-end:" + std::to_string(valueEnd)));

        switch (ScriptUtils::findTableSorted(keyword, ScriptUtils::objectTags, ScriptUtils::SCRIPTOBJ_TAG_QTY - 1))
        {
        case ScriptUtils::SCRIPTOBJ_TAG_CATEGORY:
            obj->m_category = objTree(obj->m_type)->findCategory(value);
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_SUBSECTION:
            objSubsection = value;
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_DESCRIPTION:
            objDescription = value;
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_DUPEITEM:
            // if dupeitem is not numerical, metti string normale; metti tutte le altre to lower
            if (isStringNumericHex(value))
                obj->m_dupeItem = ScriptUtils::numericalStrFormattedAsSphereInt(value);
            else
            {
                strToLower(value);
                obj->m_dupeItem = value;
            }

            // When parsing is completed, overwrite Category and Subsection for dupe items: they will have the same as the main/original item.
            // Also the name will be inherited.
            m_scriptsDupeItems.push_back(obj);
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_DUPELIST:
            // If we store now the parent for each dupe item, we can retrieve them later much quickly (instead of looping through all of the items)
            m_scriptsDupeParents.push_back(obj);
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_DEFNAME:
            strToLower(value);
            objDefname = value;
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_ID:
            if (ignoreTrigger || !obj->m_ID.empty())
                break;
            if (isStringNumericHex(value))
            {
                objID = ScriptUtils::numericalStrFormattedAsSphereInt(value);
            }
            else
            {
                strToLower(value);
                objID = value;
            }
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_COLOR:
            if (ignoreTrigger)
                break;
            obj->m_color = value;
            break;
        case ScriptUtils::SCRIPTOBJ_TAG_NAME:
            if (ignoreTrigger)
                break;
            objName = value;
            break;
        /*
        case ScriptUtils::SCRIPTOBJ_TAG_GROUP:
        {
            if (bIgnore)
                break;
            if (this->m_bType == SCRIPTOBJ_TYPE_AREA)
                this->m_csSubsection = csValue;
            break;
        }
        case ScriptUtils::SCRIPTOBJ_TAG_P:
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
        case ScriptUtils::SCRIPTOBJ_TAG_POINT:
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

    if (!obj->m_dupeItem.empty())
        goto GOTO_DUPEITEM;

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
        else if (obj->m_subsection->m_subsectionName != SCRIPTSUBSECTION_NONE_NAME)
            obj->m_description = obj->m_subsection->m_subsectionName;
    }
    else
    {
        if ( (objDescription.length() == 1) && (objDescription[0] == '@') )
        {
            if (!objName.empty())
                obj->m_description = objName;
            else if (obj->m_subsection->m_subsectionName != SCRIPTSUBSECTION_NONE_NAME)
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
        {
            strToLower(objArgument);
            obj->m_defname = objArgument;
        }

        if (!objID.empty())
            obj->m_ID = objID;
        // else:
        //  -> m_display will be assigned after we loaded all the scripts, in a second time

        if (obj->m_type == SCRIPTOBJ_TYPE_ITEM)
            m_scriptsChildItems.push_back(obj);
        else if (obj->m_type == SCRIPTOBJ_TYPE_CHAR)
            m_scriptsChildChars.push_back(obj);
    }
    else                    // It's an ID.
    {
        obj->m_baseDef = true;

        if (!objDefname.empty())
            obj->m_defname = objDefname;

        //if (!objID.empty())    // There's an "override" for the ID.
        //    obj->m_ID = objID;
        //else
            obj->m_ID = ScriptUtils::numericalStrFormattedAsSphereInt(objIDHeader);

        obj->m_display = ScriptUtils::strToSphereInt(obj->m_ID);
    }

}

