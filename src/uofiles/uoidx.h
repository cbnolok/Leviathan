#ifndef UOIDX_H
#define UOIDX_H

#include <string>


class UOIdx
{
public:
    static unsigned int getLookup(std::string path_idx, unsigned int id);
};

#endif // UOIDX_H
