#ifndef UOIDX_H
#define UOIDX_H

#include <string>


namespace uocf
{


struct UOIdx
{
    struct Entry
    {
        unsigned int lookup;
        unsigned int size;
        unsigned int unknown;
    };

    UOIdx(std::string idxPath);
    bool getLookup(unsigned int id, Entry *idxEntry);
    std::string m_idxPath;

    static bool getLookup(const std::string& idxPath, unsigned int id, Entry *idxEntry);
};


}

#endif // UOIDX_H
