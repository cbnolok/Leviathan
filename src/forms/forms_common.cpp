#include "forms_common.h"
#include "../settings/clientprofile.h"
#include "../globals.h"


std::string getClientWindowNameFragment()
{
    const ClientProfile* cp = getLoadedClientProfile();
    if (cp)
        return cp->m_clientWindowName;
    return {};
}
