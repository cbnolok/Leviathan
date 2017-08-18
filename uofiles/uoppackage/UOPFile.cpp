#include "UOPFile.h"
#include <cstring> // for memset

#include "zlib.h"

#define ADDERROR(_x_) errorHandler.m_errorQueue.push_front(_x_)


namespace uoppackage
{


int UOPFile::getIndex() const {
    return m_index;
}
unsigned long long UOPFile::getDataBlockAddress() const {
    return m_dataBlockAddress;
}
int UOPFile::getDataBlockLength() const {
    return m_dataBlockLength;
}
int UOPFile::getCompressedSize() const {
    return m_compressedSize;
}
int UOPFile::getDecompressedSize() const {
    return m_decompressedSize;
}
unsigned long long UOPFile::getFileHash() const {
    return m_fileHash;
}
unsigned int UOPFile::getDataBlockHash() const {
    return m_dataBlockHash;
}
CompressionFlag UOPFile::getCompression() const {
    return m_compression;
}
//std::string UOPFile::FileName() const {
//return m_FileName;
//}


UOPFile::UOPFile() :
    size(34), m_index(-1)
{
}

UOPFile::UOPFile(int index) :
    size(34), m_index(index)
{
}

//UOPFile::~UOPFile()
//{
//}

bool UOPFile::read(std::ifstream& fin)
{
    //m_Parent = parent;

    // Read file's header
    fin.read( reinterpret_cast<char*>(&m_dataBlockAddress), 8);
    fin.read( reinterpret_cast<char*>(&m_dataBlockLength), 4);
    fin.read( reinterpret_cast<char*>(&m_compressedSize), 4);
    fin.read( reinterpret_cast<char*>(&m_decompressedSize), 4);
    fin.read( reinterpret_cast<char*>(&m_fileHash), 8);
    fin.read( reinterpret_cast<char*>(&m_dataBlockHash), 4);

    short comprFlag;
    fin.read( reinterpret_cast<char*>(&comprFlag), 2);

    switch ( comprFlag )
    {
        case 0x0: m_compression = CompressionFlag::none; break;
        case 0x1: m_compression = CompressionFlag::zlib; break;
        default:
            ADDERROR("Unsupported compression type " + std::to_string(comprFlag));
            return false;
    }
    return true;
}

bool UOPFile::unpack(std::ifstream &fin, char* &result, size_t &resultSize) const
{
    fin.seekg( m_dataBlockAddress + m_dataBlockLength, std::ios_base::beg);

    switch ( m_compression )
    {
        case CompressionFlag::zlib:
        {
            Byte* sourceData = (Byte*) malloc(m_compressedSize);
            fin.read(reinterpret_cast<char*>(sourceData), m_compressedSize * sizeof(Byte));
            Byte* destData = (Byte*) malloc(m_decompressedSize);
            memset(destData,0,m_decompressedSize);
            uLongf destLength = m_decompressedSize;

            int z_result = ::uncompress( destData, &destLength, sourceData, m_compressedSize);

            bool success = true;
            switch (z_result)
            {
            case Z_STREAM_END:
                ADDERROR( "Zlib: Stream end!");
                success = false;   break;
            case Z_NEED_DICT:
                ADDERROR( "Zlib: Need dict!");
                success = false;   break;
            case Z_ERRNO:
                ADDERROR( "Zlib: errno!");
                success = false;   break;
            case Z_STREAM_ERROR:
                ADDERROR( "Zlib: Stream Error!");
                success = false;   break;
            case Z_DATA_ERROR:
                ADDERROR( "Zlib: Data error!");
                success = false;   break;
            case Z_MEM_ERROR:
                ADDERROR( "Zlib: Memory error!");
                success = false;   break;
            case Z_BUF_ERROR:
                ADDERROR( "Zlib: Buffer error!");
                success = false;   break;
            }

            if ( !success && (destLength != m_decompressedSize) )
            {
                ADDERROR("Different decompressed size!");
                success = true;
            }

            result = reinterpret_cast<char*>(destData);
            resultSize = (size_t)destLength;

            free(sourceData);

            return success;
        }
        case CompressionFlag::none:
        {
            result = (char*) malloc(m_compressedSize);
            fin.read(result, m_compressedSize * sizeof(char));
            resultSize = m_compressedSize;
            return true;
        }
    }

    return false;
}

bool UOPFile::searchByHash(unsigned long long hash) const
{
    return m_fileHash == hash;
}


}
