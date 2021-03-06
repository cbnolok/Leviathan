/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
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
