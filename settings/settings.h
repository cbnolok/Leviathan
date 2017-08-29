#ifndef SETTINGS_H
#define SETTINGS_H

#include <QJsonObject>


class Settings
{
public:
    Settings();

    QJsonObject generateJsonObject();
    static Settings readJsonData();

    static const bool m_kDefault_loadDefaultProfilesAtStartup = true;
    bool m_loadDefaultProfilesAtStartup;
};

#endif // SETTINGS_H
