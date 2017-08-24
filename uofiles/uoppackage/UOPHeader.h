#ifndef _UOPHEADER_H_
#define _UOPHEADER_H_

#include <fstream>


namespace uoppackage
{


class UOPHeader
{
public:
    //const int size = 20;

public:
    UOPHeader();
    //~UOPHeader();

    bool read(std::ifstream& fin);

    int getVersion() const;
    unsigned int getMisc() const;
    unsigned long long getStartAddress() const;
    int getBlockSize() const;
    int getFileCount() const;

    const int supportedVersion;

private:
    int m_version;
    unsigned int m_misc;
    unsigned long long m_startaddress;
    int m_blocksize;
    int m_filecount;
};


}
#endif //_UOPHEADER_H_
