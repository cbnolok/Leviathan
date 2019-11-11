/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
#include "uopfile.h"

#include <cstring> // for memcpy
#include <sstream>
#include "zlib.h"

#include "uopblock.h"
#include "uophash.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


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


//--

bool UOPFile::read(std::ifstream& fin, UOPError *errorQueue)
{
    // Read file's header
    fin.read(reinterpret_cast<char*>(&m_dataBlockAddress), 8);
    fin.read(reinterpret_cast<char*>(&m_dataBlockLength), 4);
    fin.read(reinterpret_cast<char*>(&m_compressedSize), 4);
    fin.read(reinterpret_cast<char*>(&m_decompressedSize), 4);
    fin.read(reinterpret_cast<char*>(&m_fileHash), 8);
    fin.read(reinterpret_cast<char*>(&m_dataBlockHash), 4);     // adler32 hash of the compressed data?

    short comprFlag = 0;
    fin.read(reinterpret_cast<char*>(&comprFlag), 2);

    switch ( comprFlag )
    {
        case 0x0: m_compression = CompressionFlag::None; break;
        case 0x1: m_compression = CompressionFlag::ZLib; break;
        default:
            ADDERROR("UOPFile::read: Unsupported compression type " + std::to_string(comprFlag));
            return false;
    }
    return true;
}

bool UOPFile::readPackedData(std::ifstream& fin, UOPError* errorQueue)
{
    m_data.reset(new char[m_compressedSize]); //std::make_shared<char[]>(m_compressedSize);  // Will work in C++20

    fin.seekg(std::streamoff(m_dataBlockAddress + m_dataBlockLength), std::ios::beg);
    fin.read(m_data.get(), m_compressedSize);

    if (fin.bad() || fin.eof())
    {
        std::stringstream ss;
        ss << "UOPFile::readPackedData: Reading packed data from malformed file (block= " << (m_parent ? m_parent->m_index : unsigned(-1)) << ", index= " << m_index << ")";
        ADDERROR(ss.str());
        return false;
    }
    return true;
}

void UOPFile::freePackedData()
{
    m_data.reset();
}

bool UOPFile::unpack(std::shared_ptr<char[]>* decompressedData, UOPError *errorQueue)
{
    if (m_data == nullptr)
        return false;

    switch ( m_compression )
    {
        case CompressionFlag::ZLib:
        {
            //*decompressedData = std::make_shared<char[]>(m_decompressedSize); // Will work in C++20
            decompressedData->reset(new char[m_decompressedSize]);
            uLongf destLength = m_decompressedSize;

            int z_result = ::uncompress(reinterpret_cast<Bytef*>(decompressedData->get()), &destLength,
                                        reinterpret_cast<const Bytef*>(m_data.get()), m_compressedSize );

            m_decompressedSize = destLength;

            if (z_result != Z_OK)
            {
                ADDERROR("UOPFile::unpack: ZLib decompression error: " + translateZlibError(z_result));
                if (destLength != uLongf(m_decompressedSize))
                    ADDERROR("UOPFile::unpack: Compressed-decompressed size mismatch!");
                //else
                    return false;
            }

            return true;
        }

        case CompressionFlag::None:
            *decompressedData = m_data;
            return true;

        default:
            ADDERROR("UOPFile::unpack: Invalid compression flag: " + std::to_string(short(m_compression)) + " (" + std::to_string(m_fileHash) + ")");
            return false;
    }
}


//--

bool UOPFile::compressAndReplaceData(std::shared_ptr<char[]> sourceDecompressed, ZLibQuality compression, bool addDataHash, UOPError* errorQueue)
{
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPFile::compressAndReplaceData: Invalid compression level: " + std::to_string(int(compression)));
        return false;
    }

    m_compression = (compression == ZLibQuality::None) ? CompressionFlag::None : CompressionFlag::ZLib;
    if (m_compression == CompressionFlag::None)
    {
        m_compressedSize = m_decompressedSize;
        m_data = sourceDecompressed;
        m_dataBlockHash = addDataHash ? hashDataBlock(m_data.get(), m_compressedSize) : 0;
        return true;
    }

    uLongf compressedSizeTemp = uLongf(::compressBound(uLong(m_decompressedSize)));
    std::unique_ptr<char[]> compressedDataTemp = std::make_unique<char[]>(compressedSizeTemp);

    int error = ::compress2(reinterpret_cast<Bytef*>(compressedDataTemp.get()), &compressedSizeTemp,
                            reinterpret_cast<const Bytef*>(sourceDecompressed.get()), uLong(m_decompressedSize),
                            int(compression) );

    m_compressedSize = unsigned(compressedSizeTemp);
    m_data.reset(new char[compressedSizeTemp]); //= std::make_shared<char[]>(compressedSizeTemp);    // Will work in C++20
    memcpy(m_data.get(), compressedDataTemp.get(), compressedSizeTemp);    

    if (error != Z_OK)
    {
        ADDERROR("UOPFile::compressAndReplaceData: ZLib compression error: " + translateZlibError(error));
        m_fileHash = m_decompressedSize = m_compressedSize = m_dataBlockHash = 0;
        m_compression = CompressionFlag::Uninitialized;
        freePackedData();
        return false;
    }

    m_dataBlockHash = addDataHash ? hashDataBlock(m_data.get(), m_compressedSize) : 0;

    return true;
}

bool UOPFile::createFile(std::ifstream& fin, unsigned long long fileHash, ZLibQuality compression, bool addDataHash, UOPError *errorQueue)    // create file in memory
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    const std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("UOPFile::createFile: Invalid fileHash: " + strHash);
        return false;
    }
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPFile::createFile: Invalid compression level: " + std::to_string(int(compression)) + " (fileHash: " + strHash + ")");
        return false;
    }
    if (fin.bad())
    {
        ADDERROR("UOPFile::createFile: Bad filestream");
        return false;
    }
    m_added = true;

    //m_dataBlockAddress        // To be filled later, in UOPPackage::finalizeAndSave();
    //m_dataBlockLength         // To be filled later, in UOPPackage::finalizeAndSave();
    m_fileHash = fileHash;      // Hashed file name

    // Get the input file size
    const std::ifstream::pos_type curPos = fin.tellg();
    fin.seekg(0, std::ios::end);
    const std::ifstream::pos_type endPos = fin.tellg();
    fin.seekg(curPos, std::ios::beg);
    const std::streamsize finSizeToRead = endPos - curPos;

    // Write the raw file data in internal buffer
    std::shared_ptr<char[]> finData(new char[size_t(finSizeToRead)]); //std::make_shared<char[]>(size_t(finSizeToRead));  // Will work in C++20
    //fin.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    fin.read(finData.get(), finSizeToRead);

    return compressAndReplaceData(finData, compression, addDataHash, errorQueue);
}

bool UOPFile::createFile(std::ifstream& fin, const std::string& packedFileName, ZLibQuality compression, bool addDataHash, UOPError *errorQueue)  // create file in memory
{
    if (packedFileName.empty())
    {
        ADDERROR("UOPFile::createFile: Invalid packedFileName: " + packedFileName);
        return false;
    }
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPFile::createFile: Invalid compression level: " + std::to_string(int(compression)) + " (" + packedFileName + ")");
        return false;
    }

    const unsigned long long fileHash = hashFileName(packedFileName);
    return createFile(fin, fileHash, compression, addDataHash, errorQueue);
}


} // end of uopp namespace
