#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <QJsonObject>


class Settings
{
public:
    Settings();

    QJsonObject generateJsonObject();
    bool updateFromJson();

    bool m_loadDefaultProfilesAtStartup;
    std::string m_customSpawnCmd;
};

#endif // SETTINGS_H
