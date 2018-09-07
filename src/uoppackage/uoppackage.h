/**
*	Ultima Online Package (UOP) Library v3.2 by Nolok
*   cbnolok  a t  gmail.com
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

public:
    UOPPackage(unsigned int maxFilesPerBlock = 100);
    ~UOPPackage();

    bool load(const std::string &fileName, UOPError* errorQueue = nullptr);
    std::ifstream getOpenedStream();
    UOPFile* getFileByName(const std::string &filename);
    UOPFile* getFileByIndex(unsigned int block, unsigned int idx) const;
    bool searchByHash(unsigned long long hash, unsigned int &block, unsigned int &index) const;
    bool addFile(const std::string &filePath, unsigned long long fileHash, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool addFile(const std::string &filePath, const std::string &packedFileName, CompressionFlag compression, UOPError* errorQueue = nullptr);
    bool finalizeAndSave(const std::string& uopPath, UOPError* errorQueue = nullptr);

// Package header data
public:
    const std::string& getPackageName() const;
    int getVersion() const;
    unsigned int getMisc() const;
    unsigned long long getStartAddress() const;
    unsigned int getBlockSize() const;
    unsigned int getFileCount() const;

private:
    std::string m_filePath;
    int m_version;
    unsigned int m_misc;
    unsigned long long m_startAddress;
    unsigned int m_blockSize;
    unsigned int m_fileCount;

public:
    // Blocks
    unsigned int getBlocksCount() const;
    std::vector<UOPBlock*> getBlocks();
    UOPBlock* getBlock(unsigned int index);
    const UOPBlock* getBlock(unsigned int index) const;

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

private:
    std::string m_packageName;
    std::vector<UOPBlock*> m_blocks;

    // Used only when creating a package
    static const int kSupportedVersion = 5;
    unsigned int m_curBlockIdx;

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

    static const unsigned int kInvalidIdx = (unsigned int)-1;
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

}

#endif // UOPACKAGE_H
