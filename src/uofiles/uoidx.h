#ifndef UOIDX_H
#define UOIDX_H

#include <string>


class UOIdx
{
public:
    static unsigned int getLookup(std::string path_idx, unsigned int id);
    static const unsigned int kInvalidLookup = (unsigned)-1;
};

#endif // UOIDX_H
