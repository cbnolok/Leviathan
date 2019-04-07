/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
#include "uopblock.h"
#include <sstream>
#include "uophash.h"

#define ADDERROR(str) UOPError::append((str), errorQueue)


namespace uopp
{


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


unsigned int UOPBlock::searchByHash(unsigned long long hash) const
{
    for ( unsigned int i = 0; i < m_fileCount; ++i )
    {
        if ( m_files[i]->searchByHash(hash) )
            return i;
    }
    return kInvalidIdx;
}


//--

void UOPBlock::read(std::ifstream& fin, UOPError *errorQueue)
{
    // Read block's header
    fin.read(reinterpret_cast<char*>(&m_fileCount), 4);
    fin.read(reinterpret_cast<char*>(&m_nextBlockAddress), 8);

    // Read files informations (headers), i'm not decompressing their held data
    m_files.reserve(m_fileCount);
    for (unsigned int index = 0; index < m_fileCount; ++index)
    {
        UOPFile* f = new UOPFile(this, index);
        //if (!f->read(fin, errorQueue))
        //    continue;  // better to load the file even if it has some weirdness
        f->read(fin, errorQueue);
        f->m_parent = this;
        m_files.push_back(f);
    }

}

bool UOPBlock::readPackedData(std::ifstream& fin, UOPError* errorQueue)
{
    for (UOPFile* file : m_files)
    {
        if (!file->readPackedData(fin, errorQueue))
            return false;
    }

    return true;
}

void UOPBlock::freePackedData()
{
    for (UOPFile* file : m_files)
    {
        file->freePackedData();
    }
}

//--

bool UOPBlock::addFile(std::ifstream& fin, unsigned long long fileHash, CompressionFlag compression, bool addDataHash, UOPError *errorQueue)
{
    std::stringstream ssHash; ssHash << std::hex << fileHash;
    std::string strHash("0x" + ssHash.str());
    if (fileHash == 0)
    {
        ADDERROR("UOPBlock::addFile: Invalid fileHash: " + strHash);
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("UOPBlock::addFile: Invalid compression flag: " + std::to_string(short(compression)) + " (fileHash: " + strHash + ")");
        return false;
    }
    if (fin.bad())
    {
        ADDERROR("UOPBlock::addFile: Bad filestream");
        return false;
    }

    if (m_curFileIdx != 0)
        ++m_curFileIdx;
    UOPFile* file = new UOPFile(this, m_curFileIdx);
    if (! file->createFile(fin, fileHash, compression, addDataHash, errorQueue) )
    {
        delete file;
        return false;
    }

    file->m_parent = this;
    m_files.push_back(file);
    ++m_fileCount;
    return true;
}

bool UOPBlock::addFile(std::ifstream& fin, const std::string& packedFileName, CompressionFlag compression, bool addDataHash, UOPError *errorQueue)
{
    if (packedFileName.empty())
    {
        ADDERROR("UOPBlock::addFile: Invalid packedFileName: " + packedFileName);
        return false;
    }
    if (compression == CompressionFlag::Uninitialized)
    {
        ADDERROR("UOPBlock::addFile: Invalid compression flag: " + std::to_string(short(compression)) + " (" + packedFileName + ")");
        return false;
    }
    unsigned long long fileHash = hashFileName(packedFileName);
    return addFile(fin, fileHash, compression, addDataHash, errorQueue);
}


// Iterators

UOPBlock::iterator UOPBlock::end()      // past-the-end iterator (obtained when incrementing an iterator to the last item)
{
    return {this, kInvalidIdx};
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
    return {this, kInvalidIdx};
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


} // end of uopp namespace
