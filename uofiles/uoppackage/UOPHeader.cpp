#include "UOPHeader.h"

#define ADDERROR(_x_) errorHandler.m_errorQueue.push_front(_x_)


namespace uoppackage
{


UOPHeader::UOPHeader():
    supportedVersion(5),
    m_misc(0xFD23EC43), m_startaddress(0x200), m_blocksize(1000)
{
}

//UOPHeader::~UOPHeader()
//{
//}

int UOPHeader::getVersion() const {
    return m_version;
}
unsigned int UOPHeader::getMisc() const {
    return m_misc;
}
unsigned long long UOPHeader::getStartAddress() const {
    return m_startaddress;
}
int UOPHeader::getBlockSize() const {
    return m_blocksize;
}
int UOPHeader::getFileCount() const {
    return m_filecount;
}

bool UOPHeader::read(std::ifstream& fin)
{
    bool success = true;
    char MYP0[4];

    fin.read(MYP0, 4);
    if ( MYP0[0] != 'M' || MYP0[1] != 'Y' || MYP0[2] != 'P' || MYP0[3] != 0 )
    {
        ADDERROR("Invalid Mythic Package file");
        success = false;
    }

    fin.read(reinterpret_cast<char*>(&m_version), 4);

    if ( m_version > supportedVersion )
    {
        ADDERROR("Unsupported Mythic Package version");
        success = false;
    }

    fin.read(reinterpret_cast<char*>(&m_misc), 4);
    fin.read(reinterpret_cast<char*>(&m_startaddress), 8);  // address of the block
    fin.read(reinterpret_cast<char*>(&m_blocksize), 4);
    fin.read(reinterpret_cast<char*>(&m_filecount), 4);

    return success;
}


}
