/**
*	Ultima Online Package Reader v1.0 by Kons
*	find me at kons.snok@gmail.com
*
*   v2.0 and later by Nolok
*   cbnolok@gmail.com
*   Current version: 2.0.1
*/

#ifndef _UOPACKAGE_H_
#define _UOPACKAGE_H_

#include <string>
#include <vector>


namespace uoppackage
{

class UOPFile;
class UOPHeader;
class UOPBlock;

class UOPPackage
{
public:
    //UOPPackage();
    ~UOPPackage();

    static unsigned long long getHash(const char * const s);
    static unsigned long long getHash(const std::string &s);

    bool load(std::string fileName);
    UOPFile* getFileByName(const std::string &filename);
    UOPFile* getFileByIndex(int block, int idx) const;
    bool searchByHash(unsigned long long hash, int &block, int &index) const;

    UOPHeader* getHeader() const;
    std::vector<UOPBlock*> getBlocks() const;
    size_t getBlocksCount() const;

private:
    std::string m_fileName;
    UOPHeader* m_header;
    std::vector<UOPBlock*> m_blocks;
};


}

#endif //_UOPACKAGE_H_
