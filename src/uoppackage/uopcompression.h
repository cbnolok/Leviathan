#ifndef UOPCOMPRESSION_H
#define UOPCOMPRESSION_H

#include <string>

namespace uopp
{


enum class CompressionFlag : short
{
    Uninitialized = -1,
    None = 0,
    ZLib = 1
};

std::string translateZlibError(int z_result);

}

#endif // UOPCOMPRESSION_H
