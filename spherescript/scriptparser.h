#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include "../globals.h"
#include "../forms/dlg_parseprogress.h"
#include <fstream>  // for std::ifstream
#include <string>
#include <vector>


class ScriptParser : public QObject
{
    Q_OBJECT
    // Need it to be a QObject to have signals and slots, in order to send progress info to the other thread.

public:
    ScriptParser(int profileIndex);
    //~ScriptParser();
    bool loadFile(std::string filePath, bool loadingResources = false);

public slots:
    void run();

signals:
    // PP: parse progress (notify the progress status to Dlg_ParseProgress
    void notifyPPProgressMax(int max);
    void notifyPPProgressVal(int val);
    void notifyPPMessage(QString msg);

private:
    int m_profileIndex;
    int m_scriptLine;   // track the number of the line we are parsing in the script file
    std::vector<std::string> m_loadedScripts;
    std::vector<ScriptObj*> m_scriptsDupeItems; // Used to temporarily store the Dupe Items before organizing them into the correct Category and Subsection
    void parseBlock(std::ifstream &fileStream, ScriptObj *obj);   //fileStream pointing to the first line after block header
};

#endif // SCRIPTPARSER_H
