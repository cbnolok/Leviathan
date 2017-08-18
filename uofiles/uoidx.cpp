#include "uoidx.h"
#include <fstream>


unsigned int UOIdx::getLookup(std::string path_idx, unsigned int id)
{
    // Open *idx.mul
    std::ifstream fs_idx;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fs_idx.open(path_idx, std::ifstream::in | std::ifstream::binary);
    if (!fs_idx.is_open())
        return (uint32_t)-1;

    // Look up in *idx.mul for the offset of the ID in *.mul
    // - Lookup:    size=4. Is either undefined ($FFFFFFFF -1) or the file offset in *.MUL
    // - Size:      size=4. Size of the pointed block.
    // - Unk:       size=4. Unknown
    uint32_t lookup = 0;
    //uint32_t size   = 0;
    fs_idx.seekg(id * 12);
    fs_idx.read(reinterpret_cast<char*>(&lookup), sizeof(uint32_t));
    //fs_artidx.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    fs_idx.close();

    return lookup;
}
