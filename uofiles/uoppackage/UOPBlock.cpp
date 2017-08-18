#include "UOPBlock.h"


namespace uoppackage
{


int UOPBlock::getIndex() const {
    return m_index;
}
std::vector<UOPFile*> UOPBlock::getFiles() const {
    return m_files;
}
int UOPBlock::getFileCount() const {
    return m_fileCount;
}
unsigned long long UOPBlock::getNextBlockAddress() const {
    return m_nextBlockAddress;
}
bool UOPBlock::isEmpty() const {
    return (m_fileCount == 0);
}
void UOPBlock::setIndex(int index) {
    m_index = index;
}

UOPBlock::UOPBlock():
    size(12)
{
}

UOPBlock::~UOPBlock()
{
    for (auto it = m_files.begin(), end = m_files.end(); it != end; it++)
        delete *it;
}

void UOPBlock::read(std::ifstream& fin)
{
    //m_parent = parent;

    // Read block's header
    fin.read( reinterpret_cast<char*>(&m_fileCount), 4);
    fin.read( reinterpret_cast<char*>(&m_nextBlockAddress), 8);

    // Read files info, i'm not decompressing them
    m_files.reserve(m_fileCount);
    for (int index = 0; index < m_fileCount; ++index)
    {
        m_files.push_back(new UOPFile(index));
        m_files[index]->read( fin );
    }

}

int UOPBlock::searchByHash(unsigned long long hash) const
{
    for ( int i = 0; i < m_fileCount; ++i )
    {
        if ( m_files[i]->searchByHash(hash) )
            return i;
    }
    return -1;
}


}
