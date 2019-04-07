/**
*	Ultima Online Package (UOP) Library v4.0.1 by Nolok
*   cbnolok  a t  gmail.com
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 3
*   as published by the Free Software Foundation
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UOPACKAGE_H
#define UOPACKAGE_H

#include "uopblock.h"


namespace uopp
{

class UOPPackage
{
    friend class UOPFile;
    friend class UOPBlock;
    static constexpr unsigned int kMinSupportedVersion = 4;
    static constexpr unsigned int kMaxSupportedVersion = 5;

public:
    UOPPackage(unsigned int version = kMaxSupportedVersion, unsigned int maxFilesPerBlock = 100);
    ~UOPPackage();

    UOPFile* getFileByIndex(unsigned int block, unsigned int index) const;
    UOPFile* getFileByName(const std::string &filename);
    bool searchByHash(unsigned long long hash, unsigned int *block, unsigned int *index) const;

    std::ifstream getOpenedStream();
    bool load(const std::string &fileName, UOPError* errorQueue = nullptr);
    bool readPackedData(UOPError* errorQueue = nullptr);
    void freePackedData();

    bool addFile(const std::string &filePath, unsigned long long fileHash,          CompressionFlag compression, bool addDataHash, UOPError* errorQueue = nullptr);
    bool addFile(const std::string &filePath, const std::string &packedFileName,    CompressionFlag compression, bool addDataHash, UOPError* errorQueue = nullptr);
    bool finalizeAndSave(const std::string& uopPath, UOPError* errorQueue = nullptr);

// Package header data
private:
    std::string m_filePath;
    unsigned int m_version;
    unsigned int m_misc;
    unsigned long long m_startAddress;
    unsigned int m_blockSize;
    unsigned int m_fileCount;

public:
    unsigned int getVersion() const             { return m_version;     }
    unsigned int getMisc() const                { return m_misc;        }
    unsigned long long getStartAddress() const  { return m_startAddress;}
    unsigned int getBlockSize() const           { return m_blockSize;   }
    unsigned int getFileCount() const           { return m_fileCount;   }

// Package structure
private:
    std::string m_packageName;
    std::vector<UOPBlock*> m_blocks;

    // Used only when creating a package
    unsigned int m_curBlockIdx;

public:
    const std::string& getPackageName() const           { return m_packageName; }

    // Blocks
    unsigned int getBlocksCount() const                 { return unsigned(m_blocks.size()); }
    std::vector<UOPBlock*> getBlocks()                  { return m_blocks;                  }
    UOPBlock* getBlock(unsigned int index)              { return m_blocks[index];           }
    const UOPBlock* getBlock(unsigned int index) const  { return m_blocks[index];           }

    // Iterators
    template <typename PointerType> class base_iterator;
    using iterator = base_iterator<UOPFile*>;
    iterator end();         // invalid iterator (obtained when incrementing an iterator to the last item)
    iterator begin();       // iterator to first item
    iterator back_it();     // iterator to last item
    using const_iterator = base_iterator<const UOPFile*>;
    const_iterator cend() const;
    const_iterator cbegin() const;
    const_iterator cback_it() const;
};

template <typename PointerType>
class UOPPackage::base_iterator
{
protected:
    const UOPPackage* m_package;
    unsigned int m_currentBlockIdx;
    unsigned int m_currentFileIdx;

public:
    base_iterator() = delete; // would construct an invalid iterator
    base_iterator(const UOPPackage* package, unsigned int currentBlockIdx, unsigned int currentFileIdx);
    ~base_iterator() = default;
    base_iterator(const base_iterator&) = default;                  // copy constructor
    base_iterator(base_iterator&&) noexcept = default;              // move constructor
    base_iterator& operator=(const base_iterator&) = default;       // copy assignment operator
    base_iterator& operator=(base_iterator&&) noexcept = default;   // move assignment operator
    bool operator==(const base_iterator&) const;
    bool operator!=(const base_iterator&) const;
    base_iterator operator++();      // pre-increment
    base_iterator operator++(int);   // post-increment
    base_iterator operator--();      // pre-decrement
    base_iterator operator--(int);   // post-decrement
    PointerType operator*();
};


// Iterator implementation

template <typename T>
UOPPackage::base_iterator<T>::base_iterator(const UOPPackage *package, unsigned int currentBlockIdx, unsigned int currentFileIdx) :
    m_package(package), m_currentBlockIdx(currentBlockIdx), m_currentFileIdx(currentFileIdx)
{
}

template <typename T>
bool UOPPackage::base_iterator<T>::operator==(const base_iterator& it) const
{
    return ((m_package == it.m_package) && (m_currentBlockIdx == it.m_currentBlockIdx) && (m_currentFileIdx == it.m_currentFileIdx));
}

template <typename T>
bool UOPPackage::base_iterator<T>::operator!=(const base_iterator& it) const
{
    return !(*this == it);
}

template <typename T>
UOPPackage::base_iterator<T> UOPPackage::base_iterator<T>::operator++()      // pre-increment
{
    if (m_currentFileIdx < m_package->getBlock(m_currentBlockIdx)->getFilesCount() - 1)
    {
        ++m_currentFileIdx;
        return *this;
    }
    if (m_currentBlockIdx < m_package->getBlocksCount() - 1)
    {
        ++m_currentBlockIdx;
        m_currentFileIdx = 0;
        return *this;
    }
    m_currentFileIdx = m_currentBlockIdx = kInvalidIdx;
    return *this;
}

template <typename T>
UOPPackage::base_iterator<T> UOPPackage::base_iterator<T>::operator++(int)   // post-increment
{
    base_iterator oldIt = *this;
    ++(*this);  // do pre-increment
    return oldIt;
}

template <typename T>
UOPPackage::base_iterator<T> UOPPackage::base_iterator<T>::operator--()      // pre-decrement
{
    if (m_currentFileIdx > 0)
    {
        --m_currentFileIdx;
        return *this;
    }
    if (m_currentBlockIdx > 0)
    {
        --m_currentBlockIdx;
        m_currentFileIdx = 0;
        return *this;
    }
    m_currentFileIdx = m_currentBlockIdx = kInvalidIdx;
}

template <typename T>
UOPPackage::base_iterator<T> UOPPackage::base_iterator<T>::operator--(int)   // post-decrement
{
    base_iterator oldIt = *this;
    --(*this);  // do pre-increment
    return oldIt;
}

template <typename PointerType>
PointerType UOPPackage::base_iterator<PointerType>::operator*()
{
    return m_package->getBlock(m_currentBlockIdx)->getFile(m_currentFileIdx);
}


} // end of uopp namespace

#endif // UOPACKAGE_H
