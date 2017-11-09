#include "clientprofile.h"
#include "globals.h"
#include "common.h"
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>

// workaround for a bug in Qt: if a key is not existant, sometimes the returned value is null, instead of undefined
#define QJSONVAL_ISVALID(qjsonvalue) (!qjsonvalue.isUndefined() && !qjsonvalue.isNull())


ClientProfile::ClientProfile(std::string clientPath) :
        m_name("Unnamed"), m_defaultProfile(false), m_clientPath(clientPath)
{
    standardizePath(m_clientPath);
}

QJsonObject ClientProfile::generateJsonObject()
{
    // Build the json object.
    QJsonObject obj;
    obj["Name"] = m_name.c_str();
    obj["Path"] = m_clientPath.c_str();
    obj["DefaultProfile"] = m_defaultProfile;

    return obj;
}

std::vector<ClientProfile> ClientProfile::createFromJson()
{
    std::vector<ClientProfile> savedProfiles;

    QFile jsonFile;
    jsonFile.setFileName("ClientProfiles.json");
    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        appendToLog("Error: can't open ClientProfiles.json! Not existant or missing permissions");
        return savedProfiles;
    }

    const QByteArray fileContent = jsonFile.readAll();
    jsonFile.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(fileContent, &err);

    QJsonObject mainObj = doc.object();
    if (mainObj.isEmpty())
        return savedProfiles;

    QJsonObject profilesListObj = mainObj["ClientProfiles"].toObject();   // usare find?
    if (profilesListObj.isEmpty())
        return savedProfiles;

    for (auto it = profilesListObj.begin(), end = profilesListObj.end(); it != end; ++it) // for each profile
    {
        QJsonObject profileObj = profilesListObj[it.key()].toObject();
        QJsonValue val = QJsonValue::Undefined;

        val = profileObj["Path"];
        if ( ! QJSONVAL_ISVALID(val) )
        {
            appendToLog("Error loading Client Profile number " + it.key().toStdString() + ". Invalid path");
            continue;
        }

        ClientProfile profile(val.toString().toStdString());

        val = profileObj["Name"];
        if (QJSONVAL_ISVALID(val))
            profile.m_name = val.toString().toStdString();

        val = profileObj["DefaultProfile"];
        if (QJSONVAL_ISVALID(val))
            profile.m_defaultProfile = val.toBool();

        val = profileObj["Path"];
        if (QJSONVAL_ISVALID(val))
            profile.m_clientPath = val.toString().toStdString();

        savedProfiles.push_back(profile);
    }

    return savedProfiles;

}
