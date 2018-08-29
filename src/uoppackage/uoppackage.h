/**
*	Ultima Online Package (UOP) Library v3.1 by Nolok
*   cbnolok@gmail.com
*/

#ifndef UOPACKAGE_H
#define UOPACKAGE_H

#include "uoperror.h"
#include "uopcompression.h"
#include <string>
#include <vector>


namespace uopp
{

class UOPFile;
class UOPBlock;

class UOPPackage
{
    friend class UOPFile;
    friend class UOPBlock;

public:
    UOPPackage(unsigned int maxFilesPerBlock = 100);
    ~UOPPackage();

    bool load(const std::string &fileName, UOPError* errorQueue = nullptr);
    std::ifstream getOpenedStream();
    UOPFile* getFileByName(const std::string &filename);
    UOPFile* getFileByIndex(int block, int idx) const;
    bool searchByHash(unsigned long long hash, int &block, int &index) const;
    bool addFile(const std::string &filePath, unsigned long long fileHash, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool addFile(const std::string &filePath, const std::string &packedFileName, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool finalizeAndSave(const std::string& uopPath, UOPError* errorQueue = nullptr);

// Package header data
public:
    const std::string& getPackageName() const;
    int getVersion() const;
    unsigned int getMisc() const;
    long long getStartAddress() const;
    int getBlockSize() const;
    int getFileCount() const;

private:
    std::string m_filePath;
    int m_version;
    unsigned int m_misc;
    long long m_startAddress;
    int m_blockSize;
    int m_fileCount;

// Blocks
public:
    int getBlocksCount() const;
    std::vector<UOPBlock*> getBlocks();
    UOPBlock* getBlock(unsigned int index);
    const UOPBlock* getBlock(unsigned int index) const;

private:
    std::string m_packageName;
    std::vector<UOPBlock*> m_blocks;

    // Used only when creating a package
    static const int kSupportedVersion = 5;
    int m_curBlockIdx;

};


}

#endif // UOPACKAGE_H
