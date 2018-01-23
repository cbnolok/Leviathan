#include "appsettings.h"
#include "globals.h"
#include "cpputils.h"
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>

// workaround for a bug in Qt: if a key is not existant, sometimes the returned value is null, instead of undefined
#define QJSONVAL_ISVALID(qjsonvalue) (!qjsonvalue.isUndefined() && !qjsonvalue.isNull())


AppSettings::AppSettings() :
    m_loadDefaultProfilesAtStartup(true), m_customSpawnCmd(".spawn %1,%2,%3,%4,%5")
{
}

QJsonObject AppSettings::generateJsonObject()
{
    // Build the json object.
    QJsonObject obj;
    obj["LoadDefaultProfilesAtStartup"] = m_loadDefaultProfilesAtStartup;
    obj["CustomSpawnCmd"] = QString::fromStdString(m_customSpawnCmd);

    return obj;
}

bool AppSettings::updateFromJson()
{
    QFile jsonFile;
    jsonFile.setFileName("Settings.json");
    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        appendToLog("Error: can't open Settings.json! Not existant or missing permissions");
        return false;
    }

    const QByteArray fileContent = jsonFile.readAll();
    jsonFile.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent, &err);

    QJsonObject mainObj = doc.object();
    if (mainObj.isEmpty())
        return false;

    QJsonObject settingsObj = mainObj["Settings"].toObject();
    if (settingsObj.isEmpty())
        return false;

    QJsonValue val = QJsonValue::Undefined;

    val = settingsObj["LoadDefaultProfilesAtStartup"];
    if (QJSONVAL_ISVALID(val))
        m_loadDefaultProfilesAtStartup = val.toBool();

    val = settingsObj["CustomSpawnCmd"];
    if (QJSONVAL_ISVALID(val))
        m_customSpawnCmd = val.toString().toStdString();

    return true;
}

