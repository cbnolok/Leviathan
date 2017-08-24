#include "UOPPackage.h"
#include "UOPError.h"
#include "UOPBlock.h"
#include "UOPHeader.h"
#include "UOPFile.h"
#include <fstream>
#include <cstring>

#define ADDERROR errorHandler.m_errorQueue.push_front


namespace uoppackage
{


//UOPPackage::UOPPackage()
//{
//}

UOPPackage::~UOPPackage()
{
    for (size_t i = 0; i < m_blocks.size(); ++i)
        delete m_blocks[i];
}

UOPHeader* UOPPackage::getHeader() const {
    return m_header;
}
std::vector<UOPBlock*> UOPPackage::getBlocks() const {
    return m_blocks;
}
size_t UOPPackage::getBlocksCount() const {
    return m_blocks.size();
}

bool UOPPackage::load(std::string fileName)
{
    errorHandler.clearErrorQueue();
    m_fileName = fileName;

    std::ifstream fin;
    fin.open(m_fileName, std::ifstream::in | std::ifstream::binary );
    if ( !fin.is_open() )
    {
        ADDERROR("UOPackage: Cannot open " + m_fileName);
        return false;
    }

    // Reading UOP Header
    m_header = new UOPHeader;
    if ( ! m_header->read(fin) )
    {
        ADDERROR("UOPackage: error reading Header");
        return false;
    }

    fin.seekg(m_header->getStartAddress(), std::ios_base::beg);


    // Reading blocks inside the UOP file
    int index = 0;
    bool iseof = false;
    do
    {
        UOPBlock* b = new UOPBlock(index);
        b->read(fin);
        m_blocks.push_back(b);

        unsigned long long nextbl = b->getNextBlockAddress();

        if (nextbl == 0)
            iseof = true;

        fin.seekg(nextbl, std::ios_base::beg);
        index++;
    }
    while ( !iseof );

    fin.close();

    return true;
}

UOPFile* UOPPackage::getFileByIndex(int block, int idx) const
{
    //return m_blocks[block]->getFiles()[idx];

    // Making this a friend function to UOPBlock class, so we don't have to call getFiles,
    //  which creates and copies a new vector

    return m_blocks[block]->m_files[idx];
}

bool UOPPackage::searchByHash(unsigned long long hash, int& block, int& index) const
{
    int idx;
    for (size_t bl = 0; bl < m_blocks.size(); ++bl)
    {
        idx = m_blocks[bl]->searchByHash(hash);
        if( idx != -1 )
        {
            block = (int)bl;
            index = idx;
            return true;
        }
    }
    return false;
}

UOPFile* UOPPackage::getFileByName(const std::string &filename)
{
    unsigned long long hash = UOPPackage::getHash(filename);
    int block = -1, index = -1;
    if ( searchByHash(hash,block,index) )
        return getFileByIndex(block, index);
    return nullptr;
}


unsigned long long UOPPackage::getHash(const char * const s)
{
    uint_fast32_t eax, ecx, edx, ebx, esi, edi;

    const uint_fast32_t len = strlen(s);

    eax = ecx = edx = 0;
    ebx = edi = esi = (uint_fast32_t) len + 0xDEADBEEF;

    uint_fast32_t i = 0;

    for ( i = 0; i + 12 < len; i += 12 )
    {
        edi = (uint_fast32_t) ( ( s[ i + 7  ] << 24 ) | ( s[ i + 6  ] << 16 ) | ( s[ i + 5 ] << 8 ) | s[ i + 4 ] ) + edi;
        esi = (uint_fast32_t) ( ( s[ i + 11 ] << 24 ) | ( s[ i + 10 ] << 16 ) | ( s[ i + 9 ] << 8 ) | s[ i + 8 ] ) + esi;
        edx = (uint_fast32_t) ( ( s[ i + 3  ] << 24 ) | ( s[ i + 2  ] << 16 ) | ( s[ i + 1 ] << 8 ) | s[ i     ] ) - esi;

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
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
            case 12:    esi += (uint_fast32_t) s[ i + 11 ] << 24;
            case 11:    esi += (uint_fast32_t) s[ i + 10 ] << 16;
            case 10:    esi += (uint_fast32_t) s[ i + 9  ] << 8;
            case 9:     esi += (uint_fast32_t) s[ i + 8  ];
            case 8:     edi += (uint_fast32_t) s[ i + 7  ] << 24;
            case 7:     edi += (uint_fast32_t) s[ i + 6  ] << 16;
            case 6:     edi += (uint_fast32_t) s[ i + 5  ] << 8;
            case 5:     edi += (uint_fast32_t) s[ i + 4  ];
            case 4:     ebx += (uint_fast32_t) s[ i + 3  ] << 24;
            case 3:     ebx += (uint_fast32_t) s[ i + 2  ] << 16;
            case 2:     ebx += (uint_fast32_t) s[ i + 1  ] << 8;
            case 1:     ebx += (uint_fast32_t) s[ i ];
            break;
            #pragma GCC diagnostic pop
        }

        esi = ( esi ^ edi ) - ( ( edi >> 18 ) ^ ( edi << 14 ) );
        ecx = ( esi ^ ebx ) - ( ( esi >> 21 ) ^ ( esi << 11 ) );
        edi = ( edi ^ ecx ) - ( ( ecx >> 7  ) ^ ( ecx << 25 ) );
        esi = ( esi ^ edi ) - ( ( edi >> 16 ) ^ ( edi << 16 ) );
        edx = ( esi ^ ecx ) - ( ( esi >> 28 ) ^ ( esi << 4  ) );
        edi = ( edi ^ edx ) - ( ( edx >> 18 ) ^ ( edx << 14 ) );
        eax = ( esi ^ edi ) - ( ( edi >> 8  ) ^ ( edi << 24 ) );

        return ( (unsigned long long) edi << 32 ) | eax;
    }

    return ( (unsigned long long) esi << 32 ) | eax;
}


unsigned long long UOPPackage::getHash(const std::string &s)
{
    uint_fast32_t eax, ecx, edx, ebx, esi, edi;

    const uint_fast32_t len = s.length();

    eax = ecx = edx = 0;
    ebx = edi = esi = (uint_fast32_t) len + 0xDEADBEEF;

    uint_fast32_t i = 0;

    for ( i = 0; i + 12 < len; i += 12 )
    {
        edi = (uint_fast32_t) ( ( s[ i + 7  ] << 24 ) | ( s[ i + 6  ] << 16 ) | ( s[ i + 5 ] << 8 ) | s[ i + 4 ] ) + edi;
        esi = (uint_fast32_t) ( ( s[ i + 11 ] << 24 ) | ( s[ i + 10 ] << 16 ) | ( s[ i + 9 ] << 8 ) | s[ i + 8 ] ) + esi;
        edx = (uint_fast32_t) ( ( s[ i + 3  ] << 24 ) | ( s[ i + 2  ] << 16 ) | ( s[ i + 1 ] << 8 ) | s[ i     ] ) - esi;

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
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
            case 12:    esi += (uint_fast32_t) s[ i + 11 ] << 24;
            case 11:    esi += (uint_fast32_t) s[ i + 10 ] << 16;
            case 10:    esi += (uint_fast32_t) s[ i + 9  ] << 8;
            case 9:     esi += (uint_fast32_t) s[ i + 8  ];
            case 8:     edi += (uint_fast32_t) s[ i + 7  ] << 24;
            case 7:     edi += (uint_fast32_t) s[ i + 6  ] << 16;
            case 6:     edi += (uint_fast32_t) s[ i + 5  ] << 8;
            case 5:     edi += (uint_fast32_t) s[ i + 4  ];
            case 4:     ebx += (uint_fast32_t) s[ i + 3  ] << 24;
            case 3:     ebx += (uint_fast32_t) s[ i + 2  ] << 16;
            case 2:     ebx += (uint_fast32_t) s[ i + 1  ] << 8;
            case 1:     ebx += (uint_fast32_t) s[ i ];
            break;
            #pragma GCC diagnostic pop
        }

        esi = ( esi ^ edi ) - ( ( edi >> 18 ) ^ ( edi << 14 ) );
        ecx = ( esi ^ ebx ) - ( ( esi >> 21 ) ^ ( esi << 11 ) );
        edi = ( edi ^ ecx ) - ( ( ecx >> 7  ) ^ ( ecx << 25 ) );
        esi = ( esi ^ edi ) - ( ( edi >> 16 ) ^ ( edi << 16 ) );
        edx = ( esi ^ ecx ) - ( ( esi >> 28 ) ^ ( esi << 4  ) );
        edi = ( edi ^ edx ) - ( ( edx >> 18 ) ^ ( edx << 14 ) );
        eax = ( esi ^ edi ) - ( ( edi >> 8  ) ^ ( edi << 24 ) );

        return ( (unsigned long long) edi << 32 ) | eax;
    }

    return ( (unsigned long long) esi << 32 ) | eax;

}


}
