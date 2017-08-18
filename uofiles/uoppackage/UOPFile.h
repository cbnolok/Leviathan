#ifndef _UOPFILE_H_
#define _UOPFILE_H_

#include "UOPError.h"
#include "UOPCompression.h"
#include <fstream>


namespace uoppackage
{


class EXPORT UOPFile
{
public:
    UOPFile();
    UOPFile(int index);
    //~UOPFile();

    bool read(std::ifstream& fin);
    bool unpack(std::ifstream& fin, char* &result, size_t &resultSize) const;
    bool searchByHash(unsigned long long hash) const;

public:
    int getIndex() const;
    unsigned long long getDataBlockAddress() const;
    int getDataBlockLength() const;
    int getCompressedSize() const;
    int getDecompressedSize() const;
    unsigned long long getFileHash() const;
    unsigned int getDataBlockHash() const;
    CompressionFlag getCompression() const;
    //std::string getFileName() const;

public:
    const int size; //34

private:
    //UOPBlock* m_parent;
    int m_index;
    unsigned long long m_dataBlockAddress;
    unsigned int m_dataBlockLength;
    unsigned int m_compressedSize;
    unsigned int m_decompressedSize;
    unsigned long long m_fileHash;
    unsigned int m_dataBlockHash;
    CompressionFlag m_compression;
    //std::string m_fileName;
};


}
#endif
