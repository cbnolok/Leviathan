#ifndef _UOPBLOCK_H_
#define _UOPBLOCK_H_

#include "UOPError.h"
#include "UOPFile.h"
#include <vector>
#include <fstream>


namespace uoppackage
{


class EXPORT UOPBlock
{
public:
    const int size; //12

public:
    UOPBlock();
    ~UOPBlock();

    void read(std::ifstream& fin);
    int searchByHash(unsigned long long hash) const;

    int getIndex() const;
    std::vector<UOPFile*> getFiles() const;
    int getFileCount() const;
    unsigned long long getNextBlockAddress() const;
    bool isEmpty() const;

    void setIndex(int);


private:
    //UOPackage* m_parent;
    int m_index;
    std::vector<UOPFile*> m_files;
    int m_fileCount;
    unsigned long long m_nextBlockAddress;
};


}
#endif
