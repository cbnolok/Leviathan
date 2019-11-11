/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
#include "uoppackage.h"

#include <cstring>      // for strcmp
#include <exception>    // for std::logic_error
#include <iostream>
#include <sstream>

// Headers needed to get the absolute path of a file
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <cstdlib>  // for realpath
#endif

#include "uophash.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


UOPPackage::UOPPackage(unsigned int version, unsigned int maxFilesPerBlock) :
    m_version(version),
    m_misc(0xFD23EC43), m_startAddress(0),
    m_blockSize(maxFilesPerBlock), m_fileCount(0),
    m_curBlockIdx(0)
{
    if ((m_version < kMinSupportedVersion) || (m_version > kMaxSupportedVersion))
        throw std::logic_error("Trying to construct UOPPackage with unsupported version=" + std::to_string(m_version));
}

UOPPackage::~UOPPackage()
{
    for (UOPBlock *block : m_blocks)
        delete block;
}


unsigned long long UOPPackage::getDefaultStartAddress() const
{
    static constexpr unsigned int kPackageHeaderSize = 0x1C;  // Size of the header in bytes = 4+4+4+8+4+4 = 28 = 0x1C
    static constexpr unsigned int kV4FirstBlockHeaderOffset = 0x28;
    static constexpr unsigned int kV5FirstBlockHeaderOffset = 0x200;

    // Version 4: all the file data is stored after the package, block and file headers.
    // Version 5: file data stored after the block or the file header? Gotta check this better, even if it's only for documentation
    //  purpose, because UO doesn't really care where the data is stored, it only cares about the m_dataBlockAddress/m_dataBlockLength.

    if (m_version == 5)
        return kV5FirstBlockHeaderOffset;
    if (m_version == 4)
        return kV4FirstBlockHeaderOffset;

    return kPackageHeaderSize;
}


UOPFile* UOPPackage::getFileByIndex(unsigned int block, unsigned int index) const
{
    if ((m_blocks.size() < block) || (m_blocks[block]->getFilesCount() < index) )
        return nullptr;
    return m_blocks[block]->m_files[index];
}

UOPFile* UOPPackage::getFileByName(const std::string &filename)
{
    const unsigned long long hash = hashFileName(filename);
    unsigned int block = kInvalidIdx, index = kInvalidIdx;
    if ( searchByHash(hash, &block, &index) )
        return getFileByIndex(block, index);
    return nullptr;
}

bool UOPPackage::searchByHash(unsigned long long hash, unsigned int *block, unsigned int *index) const
{
    unsigned int idx;
    for (unsigned int bl = 0, sz = unsigned(m_blocks.size()); bl < sz; ++bl)
    {
        idx = m_blocks[bl]->searchByHash(hash);
        if( idx != kInvalidIdx )
        {
            *block = bl;
            *index = idx;
            return true;
        }
    }
    return false;
}


//--

std::ifstream UOPPackage::getOpenedStream()
{
    std::ifstream fin;
    fin.open(m_packageName, std::ios::in | std::ios::binary );
    return fin;
}

bool UOPPackage::load(const std::string& fileName, UOPError* errorQueue)
{
    std::ifstream fin;
    fin.open(fileName, std::ios::in | std::ios::binary );
    if ( !fin.is_open() )
    {
        ADDERROR("UOPPackage::load: Cannot open (read) (" + fileName + ")");
        return false;
    }

    // Reading UOP Header
    char MYP0[4];
    fin.read(MYP0, 4);
    if ( MYP0[0] != 'M' || MYP0[1] != 'Y' || MYP0[2] != 'P' || MYP0[3] != 0 )
    {
        ADDERROR("UOPPackage::load: Invalid Mythic Package file (" + fileName + ")");
        return false;
    }

    fin.read(reinterpret_cast<char*>(&m_version), 4);
    if ((m_version < kMinSupportedVersion) || (m_version > kMaxSupportedVersion))
    {
        ADDERROR("UOPPackage::load: Unsupported Mythic Package version (" + fileName + ")");
        return false;
    }

    m_packageName = fileName;

    fin.read(reinterpret_cast<char*>(&m_misc), 4);
    fin.read(reinterpret_cast<char*>(&m_startAddress), 8);  // address of the first block
    fin.read(reinterpret_cast<char*>(&m_blockSize), 4);
    fin.read(reinterpret_cast<char*>(&m_fileCount), 4);

    fin.seekg(std::streamoff(m_startAddress), std::ios::beg);

    // Read the blocks data inside the UOP file
    unsigned int index = 0;
    bool iseof = false;
    do
    {
        UOPBlock* b = new UOPBlock(this, index);
        b->read(fin, errorQueue);
        b->m_parent = this;
        m_blocks.push_back(b);

        const unsigned long long nextbl = b->getNextBlockAddress();

        if (nextbl == 0)
            iseof = true;

        fin.seekg(std::streamoff(nextbl), std::ios::beg);
        ++index;
    }
    while ( !iseof );

    fin.close();

    return true;
}

bool UOPPackage::readPackedData(UOPError* errorQueue)
{
    std::ifstream fin = getOpenedStream();
    if (!fin.is_open())
    {
        ADDERROR("UOPPackage::readPackedData: Can't open source package " + m_packageName);
        return false;
    }

    for (UOPBlock *block : m_blocks)
    {
        if (!block->readPackedData(fin, errorQueue))
            return false;
    }

    return true;
}

void UOPPackage::freePackedData()
{
    for (UOPBlock *block : m_blocks)
    {
        block->freePackedData();
    }
}

//--

bool UOPPackage::addFile(const std::string& filePath, unsigned long long fileHash, ZLibQuality compression, bool addDataHash, UOPError *errorQueue)
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("UOPPackage::addFile: Invalid fileHash: " + strHash);
        return false;
    }
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPPackage::addFile: Invalid compression flag: " + std::to_string(int(compression)) + " (fileHash: " + strHash + ")");
        return false;
    }

    std::ifstream fin;
    fin.open(filePath, std::ios::in | std::ios::binary );
    if ( !fin.is_open() )
    {
        ADDERROR("UOPPackage::addFile: Cannot open (read) " + filePath);
        return false;
    }

    UOPBlock* curBlock;
    if ( !m_blocks.empty() && (m_blocks[m_curBlockIdx]->getFilesCount() < m_blockSize))
    {
        curBlock = m_blocks[m_curBlockIdx];
    }
    else
    {
        curBlock = new UOPBlock(this, m_curBlockIdx);
        curBlock->m_parent = this;
        m_blocks.push_back(curBlock);
        m_curBlockIdx = unsigned(m_blocks.size()) - 1;
    }

    if (!curBlock->addFile(fin, fileHash, compression, addDataHash, errorQueue))
        return false;

    fin.close();
    return true;
}

bool UOPPackage::addFile(const std::string& filePath, const std::string& packedFileName, ZLibQuality compression, bool addDataHash, UOPError *errorQueue)
{
    if (packedFileName.empty())
    {
        ADDERROR("UOPPackage::addFile: Invalid packedFileName: " + packedFileName);
        return false;
    }
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPPackage::addFile: Invalid compression flag: " + std::to_string(int(compression)) + " (" + packedFileName + ")");
        return false;
    }

    unsigned long long fileHash = hashFileName(packedFileName);
    return addFile(filePath, fileHash, compression, addDataHash, errorQueue);
}

bool UOPPackage::recompress(ZLibQuality compression, UOPError* errorQueue)
{
    if (!isValidZLibQuality(compression))
    {
        ADDERROR("UOPFile::recompress: Invalid compression level: " + std::to_string(int(compression)));
        return false;
    }

    std::ifstream finSourceUOP;
    for (UOPFile* curFile : *this)
    {
        if (curFile->getFileHash() == 0)
            continue;   // placeholder

        if ((curFile->getCompressedSize() != 0) && !curFile->m_data)
        {
            // If the data size is != 0 but the data vector is empty, then i have read only the header of this file but not the packed data: let's fix that.
            if (!finSourceUOP.is_open())
            {
                finSourceUOP = getOpenedStream();
                if (!finSourceUOP.is_open() || finSourceUOP.bad())
                {
                    ADDERROR("UOPPackage::recompress: Error opening source package " + m_packageName);
                    return false;
                }
            }

            if (!curFile->readPackedData(finSourceUOP, errorQueue))
            {
                ADDERROR("UOPPackage::recompress: Error reading source package " + m_packageName);
                return false;
            }

            std::shared_ptr<char[]> decompressedData;
            if (!curFile->unpack(&decompressedData, errorQueue) ||
                !curFile->compressAndReplaceData(decompressedData, compression, (curFile->getDataBlockHash() != 0), errorQueue))
            {
                std::stringstream ss;
                ss << "UOPPackage::recompress: Error recompressing file hash 0x" << std::hex << curFile->getFileHash() << std::dec <<
                    " (idx=" << curFile->getIndex() << " block=" << (curFile->getParent() ? curFile->getParent()->m_index : unsigned(-1)) << ") from package " << m_packageName;
                ADDERROR(ss.str());
                return false;
            }
        }
    }
    return true;
}

bool UOPPackage::finalizeAndSave(const std::string& uopPath, UOPError* errorQueue)
{
#define WRCAST(x) reinterpret_cast<const char*>(x)

    if (uopPath.empty())
    {
        ADDERROR("UOPPackage::save: Invalid path: " + uopPath);
        return false;
    }

    if (!m_packageName.empty())
    {
        // This isn't a new file. Ensure that we aren't overwriting the source file.
        bool sameSrcDest = false;

        // We could also use C++17 filesystem library to get the absolute paths
    #ifdef _WIN32
        char srcPathBuf[MAX_PATH], destPathBuf[MAX_PATH];
        GetFullPathNameA(m_packageName.c_str(), sizeof(srcPathBuf),  srcPathBuf, nullptr);
        GetFullPathNameA(uopPath.c_str(),       sizeof(destPathBuf), destPathBuf, nullptr);
        if (strcmp(srcPathBuf, destPathBuf) == 0)
            sameSrcDest = true;
    #else
        char *srcPathBuf  = realpath(m_packageName.c_str(), nullptr);
        char *destPathBuf = realpath(uopPath.c_str(),    nullptr);
        if (strcmp(srcPathBuf, destPathBuf) == 0)
            sameSrcDest = true;
        if (srcPathBuf)  free(srcPathBuf);
        if (destPathBuf) free(destPathBuf);
    #endif

        if (sameSrcDest)
        {
            ADDERROR("UOPPackage::save: Overwriting the source file is not allowed");
            return false;
        }
    }


    std::ofstream fout;
    fout.open(uopPath, std::ios::out | std::ios::binary );
    if ( !fout.is_open() )
    {
        ADDERROR("UOPPackage::save: Cannot open (write) " + m_packageName);
        return false;
    }

    // How many files do we have in the package?
    m_fileCount = 0;
    for (const UOPBlock* block : m_blocks)
        m_fileCount += block->getFilesCount();


    /* Start of the Header Data */

    // Write UOP file header
    static const char MYP0[4] = {'M', 'Y', 'P', 0};
    fout.write(MYP0, 4);
    fout.write(WRCAST(&m_version), 4);
    fout.write(WRCAST(&m_misc), 4);

    m_startAddress = getDefaultStartAddress();
    fout.write(WRCAST(&m_startAddress), 8);

    fout.write(WRCAST(&m_blockSize), 4);
    fout.write(WRCAST(&m_fileCount), 4);

    // Add zero-padding before the start of the header data
    static constexpr char zc = 0;
    for (auto i = static_cast<unsigned long long>(fout.tellp()); i < m_startAddress; ++i)
        fout.write(&zc, 1);

    // We'll need these addresses later
    std::vector<unsigned long long> blockInfoStartAddresses;
    blockInfoStartAddresses.reserve(m_blocks.size());
    std::vector<std::streamoff> fileInfoStartAddresses;
    fileInfoStartAddresses.reserve(m_blocks.size() * m_blockSize);

    // Loop through the blocks
    fout.seekp(std::streamoff(m_startAddress));
    for (const UOPBlock *curBlock : m_blocks)
    {
        blockInfoStartAddresses.push_back(static_cast<unsigned long long>(fout.tellp()));

        // Write UOP block header
        fout.write(WRCAST(&curBlock->m_fileCount), 4);
        fout.write(WRCAST(&curBlock->m_nextBlockAddress), 8);  // i don't have this yet, just write 0.
                                                                                // also 0 is the legit value if it's the last block
        unsigned int iFile = 0;
        // Loop through all the files of this block
        for (const UOPFile *curFile : curBlock->m_files)
        {
            if (curFile->getFileHash() == 0)
                continue;   // placeholder

            fileInfoStartAddresses.push_back(fout.tellp());

            // Write data for this single UOP file
            fout.write(WRCAST(&curFile->m_dataBlockAddress), 8); // i don't have this yet, just write 0
            static constexpr unsigned int kDataBlockLength = 0;
            fout.write(WRCAST(&kDataBlockLength), 4);
            fout.write(WRCAST(&curFile->m_compressedSize), 4);
            fout.write(WRCAST(&curFile->m_decompressedSize), 4);
            fout.write(WRCAST(&curFile->m_fileHash), 8);
            fout.write(WRCAST(&curFile->m_dataBlockHash), 4);
            fout.write(WRCAST(&curFile->m_compression), 2);
            ++iFile;
        }

        // If we have a number of files in this block < of the blockSize, write empty headers.
        //  Each block must have a number of headers equal to blockSize.
        while (iFile < m_blockSize)
        {
            static constexpr char emptyFileHeader[34] = {};
            fout.write(emptyFileHeader, sizeof(emptyFileHeader));
            ++iFile;
        }
    }

    // Now update UOPBlock::m_nextBlockAddress for each block
    const std::streampos endPackageHeader = fout.tellp();
    if (blockInfoStartAddresses.size() > 1)
    {
        for (size_t i = 0, sz = blockInfoStartAddresses.size() - 1; i < sz; ++i)
        {
            fout.seekp(std::streamoff(blockInfoStartAddresses[i]) + 4);
            fout.write(WRCAST(&blockInfoStartAddresses[i + 1]), 8);
        }
    }
    fout.seekp(endPackageHeader);

    if (fout.bad())
    {
        ADDERROR("UOPPackage::save: Bad ofstream (header)");
        return false;
    }

    /* End of the Header Data */


    // Now write the actual file data separately from the info zone, and write back the m_dataBlockAddress in the file header
    std::ifstream finSourceUOP;
    unsigned int iFile = 0;
    for (UOPFile* curFile : *this)
    {
        if (curFile->getFileHash() == 0)
            continue;   // placeholder

        bool freeFileData = false;
        if ((curFile->getCompressedSize() != 0) && !curFile->m_data)
        {
            // If the data size is != 0 but the data vector is empty, then i have read only the header of this file but not the packed data: let's fix that.
            if (!finSourceUOP.is_open())
            {
                finSourceUOP = getOpenedStream();
                if (!finSourceUOP.is_open() || finSourceUOP.bad())
                {
                    ADDERROR("UOPPackage::save: Error opening source package " + m_packageName);
                    return false;
                }
            }

            if (!curFile->readPackedData(finSourceUOP, errorQueue))
            {
                ADDERROR("UOPPackage::save: Error reading source package " + m_packageName);
                return false;
            }

            freeFileData = true;
        }

        // Write UOP file raw data
        unsigned long long beforeDataPos = static_cast<unsigned long long>(fout.tellp());
        fout.write(curFile->m_data.get(), std::streamsize(curFile->getDataSize()));
        const std::streampos afterDataPos = fout.tellp();

        if (freeFileData)
        {
            // If we don't do this, at the end of the saving process we'll have the whole content of the package loaded into memory
            curFile->freePackedData();
        }

        // Write back m_dataBlockAddress
        fout.seekp(fileInfoStartAddresses[iFile]);
        fout.write(WRCAST(&beforeDataPos), 8);
        fout.seekp(afterDataPos);
        if (fout.bad())
        {
            ADDERROR("UOPPackage::save: Bad ofstream");
            return false;
        }

        ++iFile;
    }

    return true;
#undef WRCAST
}


// Iterators

UOPPackage::iterator UOPPackage::end()       // past-the-end iterator (obtained when incrementing an iterator to the last item)
{
    return {this, kInvalidIdx, kInvalidIdx};
}

UOPPackage::iterator UOPPackage::begin()     // iterator to first item
{
    if (getBlocksCount() > 0)
    {
        if (getBlock(0)->getFilesCount() > 0)
            return {this, 0, 0};
    }
    return end();
}

UOPPackage::iterator UOPPackage::back_it()     // iterator to last item
{
    unsigned int blockCount = getBlocksCount();
    if (blockCount > 0)
    {
        unsigned int fileCount = getBlock(blockCount - 1)->getFilesCount();
        if (fileCount > 0)
            return {this, blockCount - 1, fileCount - 1};
    }
    return end();
}

UOPPackage::const_iterator UOPPackage::cend() const     // past-the-end iterator (obtained when incrementing an iterator to the last item)
{
    return {this, kInvalidIdx, kInvalidIdx};
}

UOPPackage::const_iterator UOPPackage::cbegin() const   // iterator to first item
{
    if (getBlocksCount() > 0)
    {
        if (getBlock(0)->getFilesCount() > 0)
            return {this, 0, 0};
    }
    return cend();
}

UOPPackage::const_iterator UOPPackage::cback_it() const // iterator to last item
{
    unsigned int blockCount = getBlocksCount();
    if (blockCount > 0)
    {
        unsigned int fileCount = getBlock(blockCount - 1)->getFilesCount();
        if (fileCount > 0)
            return {this, blockCount - 1, fileCount - 1};
    }
    return cend();
}


} // end of uopp namespace
