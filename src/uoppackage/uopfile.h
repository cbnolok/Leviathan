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

static constexpr unsigned int kInvalidIdx = static_cast<unsigned int>(-1);


class UOPFile
{
    friend class UOPBlock;
    friend class UOPPackage;

public:
    UOPFile(UOPBlock* parent, unsigned int index = 0);
    //~UOPFile();

    bool searchByHash(unsigned long long hash) const {
        return (m_fileHash == hash);
    }

    bool read(std::ifstream& fin, UOPError* errorQueue = nullptr);
    bool readPackedData(std::ifstream& fin, UOPError* errorQueue = nullptr);
    void freePackedData();
    bool unpack(std::vector<char> *decompressedData, UOPError* errorQueue = nullptr);   // extract the file

    bool compressAndReplaceData(const std::vector<char>* sourceDecompressed,CompressionFlag compression, bool addDataHash, UOPError* errorQueue = nullptr);
    bool createFile(std::ifstream& fin, unsigned long long fileHash,        CompressionFlag compression, bool addDataHash, UOPError* errorQueue = nullptr);    // create file in memory
    bool createFile(std::ifstream& fin, const std::string& packedFileName,  CompressionFlag compression, bool addDataHash, UOPError* errorQueue = nullptr);    // create file in memory

// File structure
private:
    UOPBlock* m_parent;
    unsigned int m_index;
    unsigned long long m_dataBlockAddress;
    unsigned int m_dataBlockLength;
    unsigned int m_compressedSize;
    unsigned int m_decompressedSize;
    unsigned long long m_fileHash;
    unsigned int m_dataBlockHash;
    CompressionFlag m_compression;
    std::string m_fileName;
    std::vector<char> m_data;   // contains the compressed file data

    // Used only when creating a package
    bool m_added;

public:
    unsigned int getIndex() const                   { return m_index;               }
    unsigned long long getDataBlockAddress() const  { return m_dataBlockAddress;    }
    unsigned int getDataBlockLength() const         { return m_dataBlockLength;     }
    unsigned int getCompressedSize() const          { return m_compressedSize;      }
    unsigned int getDecompressedSize() const        { return m_decompressedSize;    }
    unsigned long long getFileHash() const          { return m_fileHash;            }
    unsigned int getDataBlockHash() const           { return m_dataBlockHash;       }
    CompressionFlag getCompression() const          { return m_compression;         }
    const std::string& getFileName() const          { return m_fileName;            } // can be empty!
    bool hasData() const                            { return !m_data.empty();       }
    const std::vector<char>* getDataVec() const     { return &m_data;               }
    std::vector<char>* getDataVec()                 { return &m_data;               }
    bool isAdded() const                            { return m_added;               }
};


}

#endif // UOPFILE_H
