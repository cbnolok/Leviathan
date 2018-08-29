#include "uoppackage.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "uophash.h"
#include "uopblock.h"
#include "uopfile.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


UOPPackage::UOPPackage(unsigned int maxFilesPerBlock) :
    m_version(kSupportedVersion),
    m_misc(0xFD23EC43), m_startAddress(0),
    m_blockSize(maxFilesPerBlock), m_fileCount(0),
    m_curBlockIdx(0)
{
}

UOPPackage::~UOPPackage()
{
    for (const auto& block : m_blocks)
        delete block;
}

const std::string& UOPPackage::getPackageName() const {
    return m_packageName;
}
int UOPPackage::getVersion() const {
    return m_version;
}
unsigned int UOPPackage::getMisc() const {
    return m_misc;
}
long long UOPPackage::getStartAddress() const {
    return m_startAddress;
}
int UOPPackage::getBlockSize() const {
    return m_blockSize;
}
int UOPPackage::getFileCount() const {
    return m_fileCount;
}
int UOPPackage::getBlocksCount() const {
    return (int)m_blocks.size();
}
std::vector<UOPBlock*> UOPPackage::getBlocks() {
    return m_blocks;
}
UOPBlock* UOPPackage::getBlock(unsigned int index) {
    return m_blocks[index];
}
const UOPBlock* UOPPackage::getBlock(unsigned int index) const {
    return m_blocks[index];
}


bool UOPPackage::load(const std::string& fileName, UOPError* errorQueue)
{
    std::ifstream fin;
    fin.open(fileName, std::ios::in | std::ios::binary );
    if ( !fin.is_open() )
    {
        ADDERROR("Cannot open (read) ( " + fileName + ")");
        return false;
    }

    // Reading UOP Header
    char MYP0[4];

    fin.read(MYP0, 4);
    if ( MYP0[0] != 'M' || MYP0[1] != 'Y' || MYP0[2] != 'P' || MYP0[3] != 0 )
    {
        ADDERROR("Invalid Mythic Package file ( " + fileName + ")");
        return false;
    }

    fin.read(reinterpret_cast<char*>(&m_version), 4);
    if ( m_version > kSupportedVersion )
    {
        ADDERROR("Unsupported Mythic Package version ( " + fileName + ")");
        return false;
    }

    m_packageName = fileName;

    fin.read(reinterpret_cast<char*>(&m_misc), 4);
    fin.read(reinterpret_cast<char*>(&m_startAddress), 8);  // address of the first block
    fin.read(reinterpret_cast<char*>(&m_blockSize), 4);
    fin.read(reinterpret_cast<char*>(&m_fileCount), 4);

    fin.seekg(m_startAddress, std::ios::beg);

    // Read the blocks data inside the UOP file
    int index = 0;
    bool iseof = false;
    do
    {
        UOPBlock* b = new UOPBlock(this, index);
        b->read(fin, errorQueue);
        m_blocks.push_back(b);

        long long nextbl = b->getNextBlockAddress();

        if (nextbl == 0)
            iseof = true;

        fin.seekg(nextbl, std::ios::beg);
        ++index;
    }
    while ( !iseof );

    fin.close();

    return true;
}

std::ifstream UOPPackage::getOpenedStream()
{
    std::ifstream fin;
    fin.open(m_packageName, std::ios::in | std::ios::binary );
    return fin;
}

UOPFile* UOPPackage::getFileByIndex(int block, int idx) const
{
    if (((int)m_blocks.size() < block) || (m_blocks[block]->getFileCount() < idx) )
        return nullptr;
    return m_blocks[block]->m_files[idx];
}

bool UOPPackage::searchByHash(unsigned long long hash, int& block, int& index) const
{
    int idx;
    for (int bl = 0, sz = (int)m_blocks.size(); bl < sz; ++bl)
    {
        idx = m_blocks[bl]->searchByHash(hash);
        if( idx != -1 )
        {
            block = bl;
            index = idx;
            return true;
        }
    }
    return false;
}

UOPFile* UOPPackage::getFileByName(const std::string &filename)
{
    unsigned long long hash = hashFileName(filename);
    int block = -1, index = -1;
    if ( searchByHash(hash,block,index) )
        return getFileByIndex(block, index);
    return nullptr;
}


//--

bool UOPPackage::addFile(const std::string& filePath, unsigned long long fileHash, CompressionFlag compression, UOPError *errorQueue)
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("Invalid fileHash for UOPPackage::addFile (" + strHash + ")");
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPPackage::addFile: " + std::to_string((short)compression) + " (" + strHash + ")");
        return false;
    }
    std::ifstream fin;
    fin.open(filePath, std::ios::in | std::ios::binary );
    if ( !fin.is_open() )
    {
        ADDERROR("Cannot open (read) " + filePath);
        return false;
    }

    UOPBlock* curBlock;
    if ( !m_blocks.empty() && (m_blocks[m_curBlockIdx]->getFileCount() < m_blockSize))
        curBlock = m_blocks[m_curBlockIdx];
    else
    {         
        curBlock = new UOPBlock(this, m_curBlockIdx);
        m_blocks.push_back(curBlock);
        m_curBlockIdx = (int)m_blocks.size() - 1;
    }
    if (!curBlock->addFile(fin, fileHash, compression, errorQueue))
        return false;
    fin.close();
    return true;
}

bool UOPPackage::addFile(const std::string& filePath, const std::string& packedFileName, CompressionFlag compression, UOPError *errorQueue)
{
    if (packedFileName.empty())
    {
        ADDERROR("Invalid packedFileName for UOPPackage::addFile (" + packedFileName + ")");
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPPackage::addFile: " + std::to_string((short)compression) + " (" + packedFileName + ")");
        return false;
    }
    unsigned long long fileHash = hashFileName(packedFileName);
    return addFile(filePath, fileHash, compression, errorQueue);
}

bool UOPPackage::finalizeAndSave(const std::string& uopPath, UOPError* errorQueue)
{
    if (uopPath.empty())
    {
        ADDERROR("Invalid path for UOPPackage::save " + uopPath);
        return false;
    }
    std::ofstream fout;
    fout.open(uopPath, std::ios::out | std::ios::binary );
    if ( !fout.is_open() )
    {
        ADDERROR("Cannot open (write) " + m_packageName);
        return false;
    }

    // How many files do we have in the package?
    m_fileCount = 0;
    for (const auto& curFile : m_blocks)
    {
        m_fileCount += curFile->getFileCount();
    }

    /* Start of the Header Data */

    // Write UOP file header
    // Size of the header in bytes = 4+4+4+8+4+4=32
    char MYP0[4] = {'M', 'Y', 'P', 0};
    fout.write(MYP0, 4);
    fout.write(reinterpret_cast<char*>(&m_version), 4);
    fout.write(reinterpret_cast<char*>(&m_misc), 4);
    m_startAddress = (long long)fout.tellp() + 8 + 4 + 4;
    fout.write(reinterpret_cast<char*>(&m_startAddress), 8);
    fout.write(reinterpret_cast<char*>(&m_blockSize), 4);
    fout.write(reinterpret_cast<char*>(&m_fileCount), 4);

    // We'll need these addresses later
    std::vector<std::streampos> blockInfoStartAddresses;
    blockInfoStartAddresses.reserve(m_blocks.size());
    std::vector<std::streampos> fileInfoStartAddresses;
    fileInfoStartAddresses.reserve(m_blocks.size()*m_blockSize);
    // Loop through the blocks
    for (const auto& curBlock : m_blocks)
    {
        blockInfoStartAddresses.push_back(fout.tellp());

        // Write UOP block header
        fout.write(reinterpret_cast<char*>(&curBlock->m_fileCount), 4);
        fout.write(reinterpret_cast<char*>(&curBlock->m_nextBlockAddress), 8);  // i don't have this yet, just write 0.
                                                                                // also 0 is the legit value if it's the last block
        int iFile = 0;
        // Loop through all the files of this block
        for (const auto& curFile : curBlock->m_files)
        {
            fileInfoStartAddresses.push_back(fout.tellp());

            // Write data for this single UOP file
            curFile->m_dataBlockAddress = 0;
            fout.write(reinterpret_cast<char*>(&curFile->m_dataBlockAddress), 8); // i don't have this yet, just write 0
            curFile->m_dataBlockLength = 0;
            fout.write(reinterpret_cast<char*>(&curFile->m_dataBlockLength), 4);
            fout.write(reinterpret_cast<char*>(&curFile->m_compressedSize), 4);
            fout.write(reinterpret_cast<char*>(&curFile->m_decompressedSize), 4);
            fout.write(reinterpret_cast<char*>(&curFile->m_fileHash), 8);
            fout.write(reinterpret_cast<char*>(&curFile->m_dataBlockHash), 4);
            fout.write(reinterpret_cast<char*>(&curFile->m_compression), 2);
            ++iFile;
        }

        // If we have a number of files in this block < of the blockSize, write empty headers.
        //  Each block must have a number of headers equal to blockSize.
        while (iFile < m_blockSize)
        {
            static const char zero[34] = {0};
            fout.write(zero, sizeof(zero));
            ++iFile;
        }
    }

    // Now update UOPBlock::m_nextBlockAddress for each block
    std::streampos endPackageHeader = fout.tellp();
    if (blockInfoStartAddresses.size() > 1)
    {
        for (size_t i = 0, sz = blockInfoStartAddresses.size() - 1; i < sz; ++i)
        {
            fout.seekp((size_t)blockInfoStartAddresses[i] + 4);
            fout.write(reinterpret_cast<char*>(&blockInfoStartAddresses[i + 1]), 8);
        }
    }
    fout.seekp(endPackageHeader);

    /* End of the Header Data */


    // Now write the actual file data separately from the info zone, and write back the m_dataBlockAddress in the file header
    unsigned int iFile = 0;
    for (const auto& curBlock : m_blocks)
    {
        for (const auto& curFile : curBlock->m_files)
        {
            // Write UOP file raw data
            long long beforeDataPos = (long long)fout.tellp();
            fout.write( curFile->m_data.data(), curFile->m_data.size());
            std::streampos afterDataPos = fout.tellp();

            // Write back m_dataBlockAddress
            fout.seekp(fileInfoStartAddresses[iFile]);
            fout.write(reinterpret_cast<char*>(&beforeDataPos), 8);
            fout.seekp(afterDataPos);

            ++iFile;
        }
    }

    // Done
    bool ret = !fout.bad();
    if (ret)
    {
        ADDERROR("Bad ofstream");
    }
    fout.close();
    return ret;
}


}
