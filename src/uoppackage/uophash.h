#ifndef UOPHASH_H
#define UOPHASH_H

#include <string>


namespace uopp
{

unsigned long long hashFileName(const char * const s) noexcept;
unsigned long long hashFileName(const std::string &s) noexcept;

// Adler32 hash for the data block
unsigned int hashDataBlock(const char * const data, size_t dataLength) noexcept;

}

#endif // UOPHASH_H
