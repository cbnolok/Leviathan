#include "uopblock.h"
#include <sstream>
#include "uophash.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


unsigned int UOPBlock::getIndex() const {
    return m_index;
}
unsigned int UOPBlock::getFilesCount() const {
    return m_fileCount;
}
UOPFile* UOPBlock::getFile(int index) const {
    return m_files[index];
}
unsigned long long UOPBlock::getNextBlockAddress() const {
    return m_nextBlockAddress;
}
bool UOPBlock::isEmpty() const {
    return (m_fileCount == 0);
}

UOPBlock::UOPBlock(UOPPackage* parent, unsigned int index) :
    m_parent(parent), m_index(index),
    m_fileCount(0), m_nextBlockAddress(0), m_curFileIdx(0)
{
}

UOPBlock::~UOPBlock()
{
    for (UOPFile* file : m_files)
        delete file;
}

void UOPBlock::read(std::ifstream& fin, UOPError *errorQueue)
{
    // Read block's header
    fin.read(reinterpret_cast<char*>(&m_fileCount), 4);
    fin.read(reinterpret_cast<char*>(&m_nextBlockAddress), 8);

    // Read files info, i'm not decompressing them
    m_files.reserve(m_fileCount);
    for (unsigned int index = 0; index < m_fileCount; ++index)
    {
        UOPFile* f = new UOPFile(this, index);
        f->read( fin, errorQueue );
        m_files.push_back(f);
    }

}

unsigned int UOPBlock::searchByHash(unsigned long long hash) const
{
    for ( unsigned int i = 0; i < m_fileCount; ++i )
    {
        if ( m_files[i]->searchByHash(hash) )
            return i;
    }
    return (unsigned int)-1;
}

bool UOPBlock::addFile(std::ifstream& fin, unsigned long long fileHash, CompressionFlag compression, UOPError *errorQueue)
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("Invalid fileHash for UOPPackage::addFile (" + strHash + ")" );
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPBlock::addFile: " + std::to_string((short)compression) + " (" + strHash + ")");
        return false;
    }
    if (fin.bad())
    {
        ADDERROR("Bad filestream for UOPBlock::addFile");
        return false;
    }

    if (m_curFileIdx != 0)
        ++m_curFileIdx;
    UOPFile* file = new UOPFile(this, m_curFileIdx);
    if (! file->createFile(fin, fileHash, compression, errorQueue) )
    {
        delete file;
        return false;
    }

    m_files.push_back(file);
    ++m_fileCount;
    return true;
}

bool UOPBlock::addFile(std::ifstream& fin, const std::string& packedFileName, CompressionFlag compression, UOPError *errorQueue)
{
    if (packedFileName.empty())
    {
        ADDERROR("Invalid packedFileName for UOPPackage::addFile (" + packedFileName +")");
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("Invalid compression flag for UOPBlock::addFile: " + std::to_string((short)compression) + " (" + packedFileName + ")");
        return false;
    }
    unsigned long long fileHash = hashFileName(packedFileName);
    return addFile(fin, fileHash, compression, errorQueue);
}


// Iterators
UOPBlock::iterator UOPBlock::end()      // past-the-end iterator (obtained when incrementing an iterator to the last item)
{
    return {this, iterator::kInvalidIdx};
}

UOPBlock::iterator UOPBlock::begin()    // iterator to first item
{
    if (getFilesCount() > 0)
        return {this, 0};
    return end();
}

UOPBlock::iterator UOPBlock::back_it()  // iterator to last item
{
    unsigned int fileCount = getFilesCount();
    if (fileCount > 0)
        return {this, fileCount - 1};
    return end();
}

UOPBlock::const_iterator UOPBlock::cend() const      // past-the-end iterator (obtained when incrementing an iterator to the last item)
{
    return {this, const_iterator::kInvalidIdx};
}

UOPBlock::const_iterator UOPBlock::cbegin() const    // iterator to first item
{
    if (getFilesCount() > 0)
        return {this, 0};
    return cend();
}

UOPBlock::const_iterator UOPBlock::cback_it() const  // iterator to last item
{
    unsigned int fileCount = getFilesCount();
    if (fileCount > 0)
        return {this, fileCount - 1};
    return cend();
}

}
