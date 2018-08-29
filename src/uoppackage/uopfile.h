#ifndef UOPFILE_H
#define UOPFILE_H

#include "uoperror.h"
#include "uopcompression.h"
#include <fstream>
#include <vector>


namespace uopp
{

class UOPBlock;
class UOPPackage;

class UOPFile
{
    friend class UOPBlock;
    friend class UOPPackage;

public:
    UOPFile(UOPBlock* parent, unsigned int index = 0);
    //~UOPFile();

    bool read(std::ifstream& fin, UOPError* errorQueue = nullptr);
    bool readData(std::ifstream& fin, UOPError* errorQueue = nullptr);
    bool unpack(std::vector<char> *decompressedData, UOPError* errorQueue = nullptr);   // extract the file
    bool searchByHash(unsigned long long hash) const;
    bool compressAndReplaceData(const std::vector<char>* sourceDecompressed, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool createFile(std::ifstream& fin, unsigned long long fileHash, CompressionFlag compression, UOPError* errorQueue = nullptr);          // create file in memory
    bool createFile(std::ifstream& fin, const std::string& packedFileName, CompressionFlag compression, UOPError* errorQueue = nullptr);    // create file in memory

public:
    int getIndex() const;
    long long getDataBlockAddress() const;
    int getDataBlockLength() const;
    unsigned int getCompressedSize() const;
    unsigned int getDecompressedSize() const;
    unsigned long long getFileHash() const;
    unsigned int getDataBlockHash() const;
    CompressionFlag getCompression() const;
    const std::string& getFileName() const;     // can be empty!
    const std::vector<char>* getDataVec() const;
    std::vector<char>* getDataVec();
    bool isAdded() const;

private:
    UOPBlock* m_parent;
    int m_index;
    long long m_dataBlockAddress;
    int m_dataBlockLength;
    unsigned int m_compressedSize;
    unsigned int m_decompressedSize;
    unsigned long long m_fileHash;
    unsigned int m_dataBlockHash;
    CompressionFlag m_compression;
    std::string m_fileName;
    std::vector<char> m_data;   // contains the compressed file data

    // Used only when creating a package
    bool m_added;

    enum class ZLibQuality
    {
        None		= 0,
        Speed		= 1,
        Medium      = 5,
        Best		= 9//,
        //Default	= -1,
    };
};


}

#endif // UOPFILE_H
