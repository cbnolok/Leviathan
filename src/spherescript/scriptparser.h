#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include <QObject>
#include <fstream>  // for std::ifstream
#include <string>
#include <vector>
#include <deque>


class next_line_view_cursor;
class ScriptObj;


class ScriptParser : public QObject
{
    Q_OBJECT
    // Need it to be a QObject to have signals and slots, in order to send progress info to the other thread.

signals:
    // TP: Task Progress (notify the progress status to SubDlg_TaskProgress)
    void notifyTPProgressMax(int max);
    void notifyTPProgressVal(int val);
    void notifyTPMessage(QString msg);
    void finished();

public slots:
    void run();

public:
    ScriptParser(int profileIndex);
    //~ScriptParser();
    bool loadFile(int fileIndex, bool loadingResources = false);

private:
    int m_profileIndex;
    int m_scriptLine;   // track the number of the line we are parsing in the script file

    std::vector<std::string> m_loadedScripts;
    std::deque<ScriptObj*> m_scriptsDupeParents;   // Used to temporarily store the parent items (which has DUPELIST property) of dupe items (having DUPEITEM prop)
    std::deque<ScriptObj*> m_scriptsDupeItems;     // Used to temporarily store the Dupe Items before organizing them into the correct Category and Subsection
    std::deque<ScriptObj*> m_scriptsChildItems;
    std::deque<ScriptObj*> m_scriptsChildChars;

    void parseBlock(next_line_view_cursor *cursor, ScriptObj *obj);   //fileStream pointing to the first line after block header
};

#endif // SCRIPTPARSER_H
