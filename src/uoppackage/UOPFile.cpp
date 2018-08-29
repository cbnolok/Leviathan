#include "uopfile.h"

#include <cstring> // for memset
#include <sstream>
#include "zlib.h"

#include "uoperror.h"
#include "uophash.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


int UOPFile::getIndex() const {
    return m_index;
}
long long UOPFile::getDataBlockAddress() const {
    return m_dataBlockAddress;
}
int UOPFile::getDataBlockLength() const {
    return m_dataBlockLength;
}
unsigned int UOPFile::getCompressedSize() const {
    return m_compressedSize;
}
unsigned int UOPFile::getDecompressedSize() const {
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
const std::string& UOPFile::getFileName() const {
    return m_fileName;
}
const std::vector<char>* UOPFile::getDataVec() const {
    return &m_data;
}
std::vector<char>* UOPFile::getDataVec() {
    return &m_data;
}
bool UOPFile::isAdded() const {
    return m_added;
}


UOPFile::UOPFile(UOPBlock *parent, unsigned int index) :
    m_parent(parent), m_index(index),
    m_dataBlockAddress(0), m_dataBlockLength(0), m_compressedSize(0), m_decompressedSize(0),
    m_fileHash(0), m_dataBlockHash(0), m_compression(CompressionFlag::Uninitialized),
    m_added(false)
{
}

//UOPFile::~UOPFile()
//{
//}

bool UOPFile::read(std::ifstream& fin, UOPError *errorQueue)
{
    //m_Parent = parent;

    // Read file's header
    fin.read(reinterpret_cast<char*>(&m_dataBlockAddress), 8);
    fin.read(reinterpret_cast<char*>(&m_dataBlockLength), 4);
    fin.read(reinterpret_cast<char*>(&m_compressedSize), 4);
    fin.read(reinterpret_cast<char*>(&m_decompressedSize), 4);
    fin.read(reinterpret_cast<char*>(&m_fileHash), 8);
    fin.read(reinterpret_cast<char*>(&m_dataBlockHash), 4);

    short comprFlag = 0;
    fin.read(reinterpret_cast<char*>(&comprFlag), 2);

    switch ( comprFlag )
    {
        case 0x0: m_compression = CompressionFlag::None; break;
        case 0x1: m_compression = CompressionFlag::ZLib; break;
        default:
            ADDERROR("Unsupported compression type " + std::to_string(comprFlag));
            return false;
    }
    return true;
}

bool UOPFile::readData(std::ifstream& fin, UOPError*)
{
    m_data.resize(m_compressedSize);
    memset(m_data.data(), 0, m_compressedSize);

    fin.seekg(m_dataBlockAddress + m_dataBlockLength, std::ios_base::beg);
    fin.read(m_data.data(), m_compressedSize * sizeof(char));
    return !fin.bad();
}

bool UOPFile::unpack(std::vector<char>* decompressedData, UOPError *errorQueue)
{
    if (m_data.empty())
        return false;

    switch ( m_compression )
    {
        case CompressionFlag::ZLib:
        {
            decompressedData->resize(m_decompressedSize);
            uLongf destLength = m_decompressedSize;

            int z_result = ::uncompress(reinterpret_cast<Bytef*>(decompressedData->data()), &destLength,
                                        reinterpret_cast<const Bytef*>(m_data.data()), m_compressedSize );

            bool success = true;
            if (z_result != Z_OK)
            {
                ADDERROR(translateZlibError(z_result));
                if (destLength != (uLongf)m_decompressedSize)
                    ADDERROR("ZLib: Different decompressed size!");
                //else
                    success = false;
            }
            //resultSize = (size_t)destLength;
            return success;
        }

        case CompressionFlag::None:
            *decompressedData = m_data;
            return true;

        default:
            return false;
    }
}

bool UOPFile::searchByHash(unsigned long long hash) const
{
    return (m_fileHash == hash);
}


bool UOPFile::compressAndReplaceData(const std::vector<char>* sourceDecompressed, CompressionFlag compression, UOPError* errorQueue)
{
    if (compression == CompressionFlag::None)
    {
        m_data = *sourceDecompressed;
        return true;
    }
    if (compression != CompressionFlag::ZLib)
        return false;

    m_data.clear();
    m_data.resize(::compressBound((uLong)sourceDecompressed->size()));

    uLongf compressedSizeTemp = (uLongf)m_data.size();
    int error = ::compress2(reinterpret_cast<Bytef*>(m_data.data()), &compressedSizeTemp,
                            reinterpret_cast<const Bytef*>(sourceDecompressed->data()), (uLong)sourceDecompressed->size(),
                            (int)ZLibQuality::Speed );
    m_compressedSize = (unsigned int)compressedSizeTemp;

    if (error != Z_OK)
    {
        std::stringstream ssErr; ssErr << "ZLib compression error number " << error << ". Aborting.";
        ADDERROR(ssErr.str());
        m_fileHash = m_decompressedSize = m_compressedSize = 0;
        m_compression = CompressionFlag::Uninitialized;
        m_data.clear();
        m_data.shrink_to_fit();
        return false;
    }
    return true;
}

bool UOPFile::createFile(std::ifstream& fin, unsigned long long fileHash, CompressionFlag compression, UOPError *errorQueue)    // create file in memory
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("Invalid fileHash for UOPPackage::createFile (" + strHash + ")");
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPBlock::createFile: " + std::to_string((short)compression) + " (" + strHash + ")");
        return false;
    }
    if (fin.bad())
    {
        ADDERROR("Bad filestream for UOPFile::createFile");
        return false;
    }
    m_added = true;

    //m_dataBlockAddress        // To be filled later, in UOPPackage::updateUOPData();
    //m_dataBlockLength         // To be filled later, in UOPPackage::updateUOPData();
    //m_compressedSize          // To be filled later, in this function
    //m_decompressedSize        // To be filled later, in this function
    m_fileHash = fileHash;      // Hashed file name
    //m_dataBlockHash           // Adler32 "checksum" for the data, not really needed
    m_compression = compression;  // Is the data compressed using ZLib?

    // Get the input file size
    std::ifstream::pos_type curPos = fin.tellg();
    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type endPos = fin.tellg();
    fin.seekg(curPos, std::ios_base::beg);
    m_decompressedSize = (unsigned int)(endPos - curPos);

    // Write the raw file data in internal buffer
    m_data.clear();
    m_data.resize(m_decompressedSize);  // don't use reserve, or ifstream.read won't work!
    //fin.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    fin.read(m_data.data(), m_decompressedSize);

    m_compressedSize = m_decompressedSize;

    if (compression == CompressionFlag::ZLib)
    {
        // I need to compress the data
        std::vector<char> dataCompressed;
        dataCompressed.resize(m_decompressedSize);  // don't use reserve, or ifstream.read won't work!
        uLongf compressedSizeTemp = m_compressedSize;
        int error = ::compress2(reinterpret_cast<Bytef*>(dataCompressed.data()), &compressedSizeTemp,
                                reinterpret_cast<const Bytef*>(m_data.data()), m_decompressedSize,
                                (int)ZLibQuality::Speed );
        m_compressedSize = (unsigned int)compressedSizeTemp;

        if (error != Z_OK)
        {
            std::stringstream ssErr; ssErr << "ZLib compression error number " << error << ". Aborting.";
            ADDERROR(ssErr.str());
            m_fileHash = m_decompressedSize = m_compressedSize = 0;
            m_compression = CompressionFlag::Uninitialized;
            m_data.clear();
            m_data.shrink_to_fit();
            return false;
        }

        // Put the compressed data in my internal vector
        m_data = dataCompressed;
        m_data.resize(m_compressedSize);
    }

    return true;
}

bool UOPFile::createFile(std::ifstream& fin, const std::string& packedFileName, CompressionFlag compression, UOPError *errorQueue)  // create file in memory
{
    if (packedFileName.empty())
    {
        ADDERROR("UOPPackage: invalid packedFileName for UOPPackage::createFile (" + packedFileName + ")");
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPBlock::createFile: " + std::to_string((short)compression) + " (" + packedFileName + ")");
        return false;
    }

    unsigned long long fileHash = hashFileName(packedFileName);
    return createFile(fin, fileHash, compression, errorQueue);
}


}
