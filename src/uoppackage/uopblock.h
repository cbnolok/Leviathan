#ifndef UOPBLOCK_H
#define UOPBLOCK_H

#include "uoperror.h"
#include "uopcompression.h"
#include <vector>
#include <fstream>


namespace uopp
{

class UOPFile;
class UOPPackage;

class UOPBlock
{
    friend class UOPFile;
    friend class UOPPackage;

public:
    UOPBlock(UOPPackage* parent, int index = 0);
    ~UOPBlock();

    void read(std::ifstream& fin, UOPError* errorQueue = nullptr);
    int searchByHash(unsigned long long hash) const;
    bool addFile(std::ifstream& fin, unsigned long long fileHash, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool addFile(std::ifstream& fin, const std::string& packedFileName, CompressionFlag compression, UOPError* errorQueue = nullptr);

public:
    int getIndex() const;
    int getFileCount() const;
    UOPFile* getFile(int index) const;
    long long getNextBlockAddress() const;
    bool isEmpty() const;

private:
    UOPPackage* m_parent;
    int m_index;
    int m_fileCount;
    std::vector<UOPFile*> m_files;
    long long m_nextBlockAddress;

    // Used only when creating a package
    int m_curFileIdx;
};


}

#endif // UOPBLOCK_H
