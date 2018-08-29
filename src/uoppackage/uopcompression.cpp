#include "uopcompression.h"
#include "zlib.h"


namespace uopp
{

std::string translateZlibError(int z_result)
{
    switch (z_result)
    {
    case Z_OK:
        return "ZLib: ok!";
    case Z_STREAM_END:
        return "ZLib: Stream end!";
    case Z_NEED_DICT:
        return "ZLib: Need dict!";
    case Z_ERRNO:
        return "ZLib: errno!";
    case Z_STREAM_ERROR:
        return "ZLib: Stream Error!";
    case Z_DATA_ERROR:
        return "ZLib: Data error!";
    case Z_MEM_ERROR:
        return "ZLib: Memory error!";
    case Z_BUF_ERROR:
        return "ZLib: Buffer error!";
    }
    return std::string();
}

}
