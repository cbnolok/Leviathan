/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
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


enum class ZLibQuality
{
    Uninitialized = -1,
    None = 0,
    Speed = 1,
    Medium = 6,
    Best = 9
};


bool isValidZLibQuality(ZLibQuality compression);
std::string translateZlibError(int z_result);


}

#endif // UOPCOMPRESSION_H
