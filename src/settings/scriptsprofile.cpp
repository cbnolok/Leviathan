#include "scriptsprofile.h"
#include "globals.h"
#include "common.h"
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>


ScriptsProfile::ScriptsProfile(std::string scriptsPath) :
        m_name("Unnamed"), m_defaultProfile(false), m_scriptsPath(scriptsPath), m_useSpheretables(false)
{
    standardizePath(m_scriptsPath);
}

QJsonObject ScriptsProfile::generateJsonObject()
{
    // Build the json object.
    QJsonObject obj;
    obj["Name"] = m_name.c_str();
    obj["DefaultProfile"] = m_defaultProfile;
    obj["Path"] = m_scriptsPath.c_str();
    obj["LoadFromSpheretables"] = m_useSpheretables;

    if (!m_useSpheretables)
    {
        QJsonArray scriptsToLoad;
        for (size_t i = 0; i < m_scriptsToLoad.size(); ++i)
            scriptsToLoad.append(m_scriptsToLoad[i].c_str());

        obj["ScriptsToLoad"] = scriptsToLoad;
    }

    return obj;
}

std::vector<ScriptsProfile> ScriptsProfile::readJsonData()
{
    std::vector<ScriptsProfile> savedProfiles;

    QFile jsonFile;
    jsonFile.setFileName("ScriptsProfiles.json");
    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        appendToLog("Error: can't open ScriptsProfiles.json! Not existant or missing permissions");
        return savedProfiles;
    }

    const QByteArray fileContent = jsonFile.readAll();
    jsonFile.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent, &err);

    QJsonObject mainObj = doc.object();
    if (mainObj.isEmpty())
        return savedProfiles;

    QJsonObject profilesListObj = mainObj["ScriptsProfiles"].toObject();   // usare find?
    if (profilesListObj.isEmpty())
        return savedProfiles;

    for (auto it = profilesListObj.begin(), end = profilesListObj.end(); it != end; ++it) // for each profile
    {
        QJsonObject profileObj = profilesListObj[it.key()].toObject();
        QJsonValue val = QJsonValue::Undefined;

        val = profileObj["Path"];
        if (val == QJsonValue::Undefined)
        {
            appendToLog("Error loading profile number " + it.key().toStdString() + ". Invalid path");
            continue;
        }

        ScriptsProfile profile(val.toString().toStdString());

        val = profileObj["Name"];
        if (val != QJsonValue::Undefined)
            profile.m_name = val.toString().toStdString();

        val = QJsonValue::Undefined;
        val = profileObj["DefaultProfile"];
        if (val != QJsonValue::Undefined)
            profile.m_defaultProfile = val.toBool();

        val = QJsonValue::Undefined;
        val = profileObj["LoadFromSpheretables"];
        if (val != QJsonValue::Undefined)
            profile.m_useSpheretables = val.toBool();

        val = QJsonValue::Undefined;
        val = profileObj["Path"];
        if (val != QJsonValue::Undefined)
            profile.m_scriptsPath = val.toString().toStdString();

        if (!profile.m_useSpheretables)
        {
            // load from a QJsonArray the script file list
            QJsonArray jsonScriptList = profileObj["ScriptsToLoad"].toArray();
            for (int i = 0; i < jsonScriptList.size(); ++i)
                profile.m_scriptsToLoad.push_back(jsonScriptList[i].toString().toStdString());
        }

        savedProfiles.push_back(profile);
    }

    return savedProfiles;
    //QJsonObject sett3 = sett2.value(QString("loyaltyCard")).toObject();
}
