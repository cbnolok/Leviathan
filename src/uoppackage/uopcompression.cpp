/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
#include "uopcompression.h"
#include "zlib.h"


namespace uopp
{


std::string translateZlibError(int z_result)
{
    switch (z_result)
    {
    case Z_OK:
        return "Ok!";
    case Z_STREAM_END:
        return "Stream end!";
    case Z_NEED_DICT:
        return "Need dict!";
    case Z_ERRNO:
        return "Errno!";
    case Z_STREAM_ERROR:
        return "Stream Error!";
    case Z_DATA_ERROR:
        return "Data error!";
    case Z_MEM_ERROR:
        return "Memory error!";
    case Z_BUF_ERROR:
        return "Buffer error!";
    }
    return {};
}


} // end of uopp namespace
