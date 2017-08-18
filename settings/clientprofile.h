#ifndef CLIENTPROFILE_H
#define CLIENTPROFILE_H

#include <string>
#include <vector>
#include <QJsonObject>

class ClientProfile
{
public:
    ClientProfile(std::string clientPath);

    int m_index;
    std::string m_name;
    bool m_defaultProfile;
    std::string m_clientPath;

    QJsonObject generateJsonObject();
    static std::vector<ClientProfile> readJsonData();
};

#endif // CLIENTPROFILE_H
