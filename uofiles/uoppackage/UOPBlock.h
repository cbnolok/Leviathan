#ifndef _UOPBLOCK_H_
#define _UOPBLOCK_H_

// i can't simply forward declare UOPPackage class because i need to declare a friend function here in UOPBlock class
#include "UOPPackage.h"
#include <vector>
#include <fstream>


namespace uoppackage
{

class UOPFile;

class UOPBlock
{
public:
    //const int size = 12;

public:
    //UOPBlock();
    UOPBlock(int blockIndex);
    ~UOPBlock();

    void read(std::ifstream& fin);
    int searchByHash(unsigned long long hash) const;

    int getIndex() const;
    std::vector<UOPFile*> getFiles() const;
    int getFileCount() const;
    unsigned long long getNextBlockAddress() const;
    bool isEmpty() const;

private:
    //UOPackage* m_parent;
    int m_index;
    std::vector<UOPFile*> m_files;
    int m_fileCount;
    unsigned long long m_nextBlockAddress;

    friend UOPFile* UOPPackage::getFileByIndex(int block, int idx) const;
};


}
#endif
