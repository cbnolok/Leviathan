#ifndef PROFILE_H
#define PROFILE_H

#include <QJsonObject>
#include <string>
#include <vector>


class ScriptsProfile
{
public:
    ScriptsProfile(std::string scriptsPath);

    QJsonObject generateJsonObject();
    static std::vector<ScriptsProfile> readJsonData();

    int m_index;
    std::string m_name;
    bool m_defaultProfile;
    std::string m_scriptsPath;
    bool m_useSpheretables;
    std::vector<std::string> m_scriptsToLoad;
};

#endif // PROFILE_H
