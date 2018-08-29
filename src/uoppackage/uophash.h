#ifndef UOPHASH_H
#define UOPHASH_H

#include <string>


namespace uopp
{

/*
class UOPHash
{
};
*/

unsigned long long hashFileName(const char * const s);
unsigned long long hashFileName(const std::string &s);

// Adler32 hash for the data block
unsigned int hashDataBlock(const char * const data, size_t dataLength);

}

#endif // UOPHASH_H
