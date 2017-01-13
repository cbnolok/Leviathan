#ifndef PROFILE_H
#define PROFILE_H

#include <QJsonObject>
#include <string>
#include <vector>


class ScriptsProfile
{
public:
    ScriptsProfile(std::string scriptsPath);
    int m_index;
    std::string m_name;
    std::string m_scriptsPath;
    bool m_defaultProfile = false;
    bool m_useSpheretables = false;
    std::vector<std::string> m_scriptsToLoad;
    void loadDataFromJson();
    QJsonObject generateJsonObject();
    static std::vector<ScriptsProfile> readJsonData();
};

#endif // PROFILE_H
