#include "uoidx.h"
#include <fstream>

namespace uocf
{

UOIdx::UOIdx(std::string idxPath) : m_idxPath(std::move(idxPath))
{
}

bool UOIdx::getLookup(unsigned int id, Entry *idxEntry)
{
    return getLookup(m_idxPath, id, idxEntry);
}

bool UOIdx::getLookup(const std::string& idxPath, unsigned int id, Entry* idxEntry)   // static
{
    // Open *idx.mul
    std::ifstream fin;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fin.open(idxPath, std::ifstream::in | std::ifstream::binary);
    if (!fin.is_open())
        return false;

    // Look up in *idx.mul for the offset of the ID in *.mul
    // - Lookup:    size=4. Is either undefined ($FFFFFFFF -1) or the file offset in *.MUL
    // - Size:      size=4. Size of the pointed block.
    // - Unk:       size=4. Unknown
    fin.seekg(id * 12);
    fin.read(reinterpret_cast<char*>(&idxEntry->lookup), 4);
    fin.read(reinterpret_cast<char*>(&idxEntry->size), 4);
    fin.read(reinterpret_cast<char*>(&idxEntry->unknown), 4);
    fin.close();

    return true;
}


}
