#ifndef CLIENTPROFILE_H
#define CLIENTPROFILE_H

#include <string>
#include <vector>
#include <QJsonObject>

class ClientProfile
{
public:
    ClientProfile(std::string clientPath);

    QJsonObject generateJsonObject();
    static std::vector<ClientProfile> createFromJson();

    int m_index;
    bool m_defaultProfile;
    std::string m_name;
    std::string m_clientPath;
    std::string m_clientWindowName;
};

#endif // CLIENTPROFILE_H
