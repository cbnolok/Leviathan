/**
* This Source Code Form is part of UOP Package library by Nolok and subject to the terms of the
* GNU General Public License version 3. More info in the file "uoppackage.h", which is part of this source code package.
*/
#include "uophash.h"

#include <cinttypes>
#include <cstring>

#if __cplusplus >= 201703L  // is C++17 enabled?
    #define FALLTHROUGH [[fallthrough]]
#else
    #define FALLTHROUGH // fall through
    /* yep, the comment appears to silence the warning with GCC, dunno for clang */
#endif


namespace uopp
{


// HashLittle3 algorithm by Jenkins, used to get the hash for the file name
unsigned long long hashFileName(const char * const s) noexcept
{
    uint_fast32_t eax, ecx, edx, ebx, esi, edi;

    const auto len = uint_fast32_t(strlen(s));

    eax = ecx = edx = 0;
    ebx = edi = esi = uint_fast32_t(len + 0xDEADBEEF);

    uint_fast32_t i = 0;

    for ( i = 0; i + 12 < len; i += 12 )
    {
        edi = uint_fast32_t( ( s[ i + 7  ] << 24 ) | ( s[ i + 6  ] << 16 ) | ( s[ i + 5 ] << 8 ) | s[ i + 4 ] ) + edi;
        esi = uint_fast32_t( ( s[ i + 11 ] << 24 ) | ( s[ i + 10 ] << 16 ) | ( s[ i + 9 ] << 8 ) | s[ i + 8 ] ) + esi;
        edx = uint_fast32_t( ( s[ i + 3  ] << 24 ) | ( s[ i + 2  ] << 16 ) | ( s[ i + 1 ] << 8 ) | s[ i     ] ) - esi;

        edx = ( edx + ebx ) ^ ( esi >> 28 ) ^ ( esi << 4 );
        esi += edi;
        edi = ( edi - edx ) ^ ( edx >> 26 ) ^ ( edx << 6 );
        edx += esi;
        esi = ( esi - edi ) ^ ( edi >> 24 ) ^ ( edi << 8 );
        edi += edx;
        ebx = ( edx - esi ) ^ ( esi >> 16 ) ^ ( esi << 16 );
        esi += edi;
        edi = ( edi - ebx ) ^ ( ebx >> 13 ) ^ ( ebx << 19 );
        ebx += esi;
        esi = ( esi - edi ) ^ ( edi >> 28 ) ^ ( edi << 4 );
        edi += ebx;
    }

    if ( len - i > 0 )
    {
        switch ( len - i )
        {
        #if defined(__GNUC__) && (__GNUC__ >= 7)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
        #endif
            case 12:    esi += uint_fast32_t( s[ i + 11 ] << 24 );  FALLTHROUGH;
            case 11:    esi += uint_fast32_t( s[ i + 10 ] << 16 );  FALLTHROUGH;
            case 10:    esi += uint_fast32_t( s[ i + 9  ] << 8  );  FALLTHROUGH;
            case 9:     esi += uint_fast32_t( s[ i + 8  ]       );  FALLTHROUGH;
            case 8:     edi += uint_fast32_t( s[ i + 7  ] << 24 );  FALLTHROUGH;
            case 7:     edi += uint_fast32_t( s[ i + 6  ] << 16 );  FALLTHROUGH;
            case 6:     edi += uint_fast32_t( s[ i + 5  ] << 8  );  FALLTHROUGH;
            case 5:     edi += uint_fast32_t( s[ i + 4  ]       );  FALLTHROUGH;
            case 4:     ebx += uint_fast32_t( s[ i + 3  ] << 24 );  FALLTHROUGH;
            case 3:     ebx += uint_fast32_t( s[ i + 2  ] << 16 );  FALLTHROUGH;
            case 2:     ebx += uint_fast32_t( s[ i + 1  ] << 8  );  FALLTHROUGH;
            case 1:     ebx += uint_fast32_t( s[ i ]            );  FALLTHROUGH;
            break;
        #if defined(__GNUC__) && (__GNUC__ >= 7)
            #pragma GCC diagnostic pop
        #endif
        }

        esi = ( esi ^ edi ) - ( ( edi >> 18 ) ^ ( edi << 14 ) );
        ecx = ( esi ^ ebx ) - ( ( esi >> 21 ) ^ ( esi << 11 ) );
        edi = ( edi ^ ecx ) - ( ( ecx >> 7  ) ^ ( ecx << 25 ) );
        esi = ( esi ^ edi ) - ( ( edi >> 16 ) ^ ( edi << 16 ) );
        edx = ( esi ^ ecx ) - ( ( esi >> 28 ) ^ ( esi << 4  ) );
        edi = ( edi ^ edx ) - ( ( edx >> 18 ) ^ ( edx << 14 ) );
        eax = ( esi ^ edi ) - ( ( edi >> 8  ) ^ ( edi << 24 ) );

        return ( static_cast<unsigned long long>(edi) << 32 ) | eax;
    }

    return ( static_cast<unsigned long long>(esi) << 32 ) | eax;
}


unsigned long long hashFileName(const std::string &s) noexcept
{
    uint_fast32_t eax, ecx, edx, ebx, esi, edi;

    const auto len = uint_fast32_t(s.length());

    eax = ecx = edx = 0;
    ebx = edi = esi = uint_fast32_t(len + 0xDEADBEEF);

    uint_fast32_t i = 0;

    for ( i = 0; i + 12 < len; i += 12 )
    {
        edi = uint_fast32_t( ( s[ i + 7  ] << 24 ) | ( s[ i + 6  ] << 16 ) | ( s[ i + 5 ] << 8 ) | s[ i + 4 ] ) + edi;
        esi = uint_fast32_t( ( s[ i + 11 ] << 24 ) | ( s[ i + 10 ] << 16 ) | ( s[ i + 9 ] << 8 ) | s[ i + 8 ] ) + esi;
        edx = uint_fast32_t( ( s[ i + 3  ] << 24 ) | ( s[ i + 2  ] << 16 ) | ( s[ i + 1 ] << 8 ) | s[ i     ] ) - esi;

        edx = ( edx + ebx ) ^ ( esi >> 28 ) ^ ( esi << 4 );
        esi += edi;
        edi = ( edi - edx ) ^ ( edx >> 26 ) ^ ( edx << 6 );
        edx += esi;
        esi = ( esi - edi ) ^ ( edi >> 24 ) ^ ( edi << 8 );
        edi += edx;
        ebx = ( edx - esi ) ^ ( esi >> 16 ) ^ ( esi << 16 );
        esi += edi;
        edi = ( edi - ebx ) ^ ( ebx >> 13 ) ^ ( ebx << 19 );
        ebx += esi;
        esi = ( esi - edi ) ^ ( edi >> 28 ) ^ ( edi << 4 );
        edi += ebx;
    }

    if ( len - i > 0 )
    {
        switch ( len - i )
        {
        #if defined(__GNUC__) && (__GNUC__ >= 7)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
        #endif
            case 12:    esi += uint_fast32_t( s[ i + 11 ] << 24 );  FALLTHROUGH;
            case 11:    esi += uint_fast32_t( s[ i + 10 ] << 16 );  FALLTHROUGH;
            case 10:    esi += uint_fast32_t( s[ i + 9  ] << 8  );  FALLTHROUGH;
            case 9:     esi += uint_fast32_t( s[ i + 8  ]       );  FALLTHROUGH;
            case 8:     edi += uint_fast32_t( s[ i + 7  ] << 24 );  FALLTHROUGH;
            case 7:     edi += uint_fast32_t( s[ i + 6  ] << 16 );  FALLTHROUGH;
            case 6:     edi += uint_fast32_t( s[ i + 5  ] << 8  );  FALLTHROUGH;
            case 5:     edi += uint_fast32_t( s[ i + 4  ]       );  FALLTHROUGH;
            case 4:     ebx += uint_fast32_t( s[ i + 3  ] << 24 );  FALLTHROUGH;
            case 3:     ebx += uint_fast32_t( s[ i + 2  ] << 16 );  FALLTHROUGH;
            case 2:     ebx += uint_fast32_t( s[ i + 1  ] << 8  );  FALLTHROUGH;
            case 1:     ebx += uint_fast32_t( s[ i ]            );  FALLTHROUGH;
            break;
        #if defined(__GNUC__) && (__GNUC__ >= 7)
            #pragma GCC diagnostic pop
        #endif
        }

        esi = ( esi ^ edi ) - ( ( edi >> 18 ) ^ ( edi << 14 ) );
        ecx = ( esi ^ ebx ) - ( ( esi >> 21 ) ^ ( esi << 11 ) );
        edi = ( edi ^ ecx ) - ( ( ecx >> 7  ) ^ ( ecx << 25 ) );
        esi = ( esi ^ edi ) - ( ( edi >> 16 ) ^ ( edi << 16 ) );
        edx = ( esi ^ ecx ) - ( ( esi >> 28 ) ^ ( esi << 4  ) );
        edi = ( edi ^ edx ) - ( ( edx >> 18 ) ^ ( edx << 14 ) );
        eax = ( esi ^ edi ) - ( ( edi >> 8  ) ^ ( edi << 24 ) );

        return ( static_cast<unsigned long long>(edi) << 32 ) | eax;
    }

    return ( static_cast<unsigned long long>(esi) << 32 ) | eax;

}


// Adler32 hash for the data block
unsigned int hashDataBlock(const char * const data, size_t dataLength) noexcept
{
    unsigned int a = 1;
    unsigned int b = 0;

    for ( size_t i = 0; i < dataLength; ++i )
    {
        a = ( a + static_cast<unsigned int>(data[i]) ) % 0xFFF1;
        b = ( b + a ) % 0xFFF1;
    }

    return ( b << 16 ) | a;
}


} // end of uopp namespace
