#include "settings.h"
#include "globals.h"
#include "common.h"
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>


Settings::Settings() :
    m_loadDefaultProfilesAtStartup(m_kDefault_loadDefaultProfilesAtStartup)
{
}

QJsonObject Settings::generateJsonObject()
{
    // Build the json object.
    QJsonObject obj;
    obj["LoadDefaultProfilesAtStartup"] = m_loadDefaultProfilesAtStartup;

    return obj;
}

Settings Settings::readJsonData()
{
    Settings savedSettings;

    QFile jsonFile;
    jsonFile.setFileName("Settings.json");
    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        appendToLog("Error: can't open Settings.json! Not existant or missing permissions");
        return savedSettings;
    }

    const QByteArray fileContent = jsonFile.readAll();
    jsonFile.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent, &err);

    QJsonObject mainObj = doc.object();
    if (mainObj.isEmpty())
        return savedSettings;

    QJsonObject settingsObj = mainObj["Settings"].toObject();   // usare find?
    if (settingsObj.isEmpty())
        return savedSettings;

    QJsonValue val = QJsonValue::Undefined;

    val = settingsObj["LoadDefaultProfilesAtStartup"];
    if (val == QJsonValue::Undefined)
        savedSettings.m_loadDefaultProfilesAtStartup = m_kDefault_loadDefaultProfilesAtStartup;

    return savedSettings;

}

