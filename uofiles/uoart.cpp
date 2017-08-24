#include "uoart.h"
#include "globals.h"
#include "uohues.h"
#include <fstream>
#include <QImage>


UOArt::UOArt(std::string clientPath) :
    m_clientPath(clientPath)
{
}

/*
UOArt::~UOArt()
{
}
*/


QImage* UOArt::drawArt(int id, int hueIndex, bool partialHue)
{
    /*
    There are three types of art images; land, static, and UO alpha
    Land images are of flat terrain.
    Static images are of items (walls, equipment, food, etc)
    UO alpha images are a sort of animated characters, they used these instead of Anim.mul in the UO alpha.

    Static and UO alpha images share the same format, while Land images use another.
    */

    unsigned lookup = UOIdx::getLookup(m_clientPath + "artidx.mul", id);
    if (lookup == (unsigned)-1)
    {
        appendToLog("Error looking up artidx.mul");
        return nullptr;
    }

    std::ifstream fs_art;
    fs_art.open(m_clientPath + "art.mul", std::ifstream::in | std::ifstream::binary);
    if (!fs_art.is_open())
    {
        appendToLog("Error loading art.mul");
        return nullptr;
    }
    fs_art.seekg(lookup);

    int_fast16_t width = 0, height = 0;
    QImage* img = nullptr;


    if (id <= 0x4000)
    {
        // Read land tile

        /*
        It's a "raw" tile, there's no offset-run encoding. The idx lookup points only to:
        WORD[1024] imageColors;

        Land images are fixed size at 44x44.
        They also have fixed transparent locations, and the format is optimized for this.
        The data stored for a tile resembles a square block, rotated 45 degrees.
        Therefore, the tile is loaded from the tip, down to the widest portion, then down to the bottom tip as opposed to a straight matrix.
        WORD imageColors, in the series:
        2, 4, 6, 8, 10 ... 40, 42, 44, 44, 42, 40 ... 10, 8, 6, 4, 2

        To read the first 22 lines, you first initialize X to 22, Y to 0, and LineWidth to 2
        Then repeat the following 22 times:
        Decrease X by 1
        Read and Draw (LineWidth) number of pixels
        Increase Y by 1
        Increase LineWidth by 2

        For the last 22 lines, do the following:
        Read and Draw (LineWidth) number of pixels
        Increase X by 1
        Increase Y by 1
        Decrease LineWidth by 2
        The resulting image is the diamond shaped tile.
        */

        width = height = 44;
        img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
        img->fill(0);

        // Algorithm from Punt's C++ Ultima SDK
        uint_fast16_t rawcolor_argb16 = 0;
        int X = 22;
        int Y = 0;
        int linewidth = 2;
        for (int temp = 0; temp < 22; ++temp)
        {
            --X;
            for (int draw = 0; draw < linewidth; ++draw)
            {
                fs_art.read(reinterpret_cast<char*>(&rawcolor_argb16), 2);
                ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                //if ( (color &0x7FF) == 0 || (color & 0x7FF)==0x7FF )
                //  continue;
                if (hueIndex != 0)
                {
                    UOHueEntry hue = g_UOHues->getHue(hueIndex);
                    color_argb16 = hue.applyToColor(color_argb16, partialHue);
                }
                ARGB32 color_argb32 = argb16_to_argb32(color_argb16);
                img->setPixel(X+draw, Y, color_argb32.getVal());
            }
            ++Y ;
            linewidth += 2;
        }

        X = 0;
        linewidth = 44;
        Y = 22;
        for (int temp = 0; temp < 22; ++temp)
        {
            for (int draw = 0; draw < linewidth; ++draw)
            {
                fs_art.read(reinterpret_cast<char*>(&rawcolor_argb16), 2);
                ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                //if ( (color &0x7FF) == 0 || (color & 0x7FF)==0x7FF )
                //  continue;
                if (hueIndex != 0)
                {
                    UOHueEntry hue = g_UOHues->getHue(hueIndex);
                    color_argb16 = hue.applyToColor(color_argb16, partialHue);
                }
                ARGB32 color_argb32 = argb16_to_argb32(color_argb16);
                img->setPixel(X+draw, Y, color_argb32.getVal());
            }
            ++X;
            ++Y;
            linewidth -= 2;
        }
    }
    else
    {
        // Read static tile

        /*
        DWORD header; // Unknown
        WORD width;
        WORD height;
        WORD[height] lookupTable;
        Offset-Run data...

        The lookup table is offset from the data start, and treats the data by 16-bits.
        To get the proper byte offset, you need to add 4 + height, then multiply by two.
        The static images are compressed with an offset-run encoding.

        Offset-Run data:
        WORD xOffset;
        WORD xRun;
        WORD[xRun] runColors;

        If xOffset is 0 and xRun is 0, the line has been completed. If not, continue reading the chunks.
        Processing the Offset-Run data is simple:
        Increase X by xOffset
        Foreach xRun
        Read and Draw the next pixel
        Increase X by 1
        End Foreach

        When the line is completed, simply reset X to 0, increase Y, and seek to the next lookup in the lookupTable array and continue.
        */

        uint_fast32_t flags = 0;
        fs_art.read(reinterpret_cast<char*>(&flags), 4);
        fs_art.read(reinterpret_cast<char*>(&width), 2);
        fs_art.read(reinterpret_cast<char*>(&height), 2);

        // Get the lookup table
        uint16_t* lookups = new uint16_t[height];
        fs_art.read(reinterpret_cast<char*>(lookups), 2*height);
        size_t datastart = fs_art.tellg();

        // Draw art
        img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
        img->fill(0);

        // Algorithm from Punt's C++ Ultima SDK
        for (int Y=0, X=0; Y < height ; ++Y)
        {
            X=0;

            fs_art.seekg(lookups[Y] * 2 + datastart);
            uint_fast16_t xOffset = 1, xRun = 1;
            while (xOffset + xRun != 0)
            {
                fs_art.read(reinterpret_cast<char*>(&xOffset), 2);
                fs_art.read(reinterpret_cast<char*>(&xRun), 2);
                if (xOffset + xRun != 0)
                {
                    X += xOffset ;
                    for (unsigned jj=0; jj < xRun; ++jj)
                    {
                        uint_fast16_t rawcolor_argb16 = 0;
                        fs_art.read(reinterpret_cast<char*>(&rawcolor_argb16), 2);
                        ARGB16 color_argb16 = ARGB16(rawcolor_argb16);
                        if (hueIndex != 0)
                        {
                            UOHueEntry hue = g_UOHues->getHue(hueIndex);
                            color_argb16 = hue.applyToColor(color_argb16, partialHue);
                        }
                        ARGB32 color_argb32 = argb16_to_argb32(color_argb16);
                        img->setPixel(X, Y, color_argb32.getVal());
                        ++X;
                    }
                }
            }
        }

        delete[] lookups;
    }

    fs_art.close();
    return img;
}
