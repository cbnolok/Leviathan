#include "uoanimmul.h"

#include <fstream>
#include <sstream>
#include <QImage>

#include "../cpputils/strings.h"
#include "uoidx.h"
#include "uohues.h"

#include "../globals.h"
#define LOG(x) appendToLog(x)


namespace uocf
{

UOAnimMUL::UOAnimMUL(const std::string &clientPath) :
    m_UOHues(nullptr), m_clientPath(clientPath)
{
    loadBodyDef();
    loadBodyConvDef();
}


//--Body.def and BodyConv.def (source inspired by UOFiddler)


bool UOAnimMUL::loadBodyDef()
{
    LOG("Loading Body.def");

    std::ifstream fileStream;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fileStream.open(m_clientPath + "Body.def", std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open())
    {
        LOG("Error opening file Body.def");
        return false;
    }

    while ( !fileStream.eof() )
    {
        std::string line;
        std::getline(fileStream, line);
        if ( fileStream.bad() )
            break;

        strTrim(line);
        if (line.empty())
            continue;
        if (line[0]=='#')   // commented line
            continue;

        // Format is: <ORIG BODY> {<NEW BODY>} <NEW HUE>
        size_t index1 = line.find('{');
        size_t index2 = line.find('}', index1 + 1);
        size_t index3 = line.find('#', index2 + 1); // get rid of eventual comments at the end of the line
        if ( (index1 == std::string::npos) || (index2 == std::string::npos) )
            continue;

        std::string param1 = line.substr(0, index1 - 1);
        std::string param2 = line.substr(index1 + 1, index2 - index1 - 1);
        std::string param3 = line.substr(index2 + 1, index3 - index2 - 1);
        strTrim(param1);
        strTrim(param2);
        strTrim(param3);

        size_t indexOf = param2.find(',');
        if (indexOf != std::string::npos)
        {
            param2 = param2.substr(0, indexOf);
            strTrim(param2);
        }

        int iParam1 = std::stoi(param1);
        int iParam2 = std::stoi(param2);
        int iParam3 = std::stoi(param3);

        std::pair<int,BodyDefEntry> k (iParam1, BodyDefEntry(iParam2, iParam3));
        m_bodyDef.insert(k);
    }

    fileStream.close();
    return true;
}


bool UOAnimMUL::loadBodyConvDef()
{
    LOG("Loading BodyConv.def");

/*
    0 - 199 = Monsters
    200 - 399 = Animals
    400 + = Humans/Elves and Equipment

    The maximum value for an index is 2048.
*/

    std::ifstream fileStream;
    // it's fundamental to open the file in binary mode, otherwise tellg and seekg won't work properly...
    fileStream.open(m_clientPath + "Bodyconv.def", std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open())
    {
        LOG("Error opening file Bodyconv.def");
        return false;
    }

    while ( !fileStream.eof() )
    {
        std::string line;
        std::getline(fileStream, line);
        if ( fileStream.bad() )
            break;

        strTrim(line);
        if (line.empty())
            continue;
        if (line[0]=='#')   // commented line
            continue;

        // Format is: <Object> <LBR expansion (anim2)> <AoS (anim3)> <SE (anim4)> <Mondain's Legacy (anim5)>
        std::vector<std::string> params;
        strSplit(line, params, " \t", true);
        if (params.size() < 5)
            continue;

        int iParams[5];
        for (int i = 0; i < 5; ++i)
            std::istringstream(params[i]) >> iParams[i];

        if (iParams[1] == 68)   // anim2
            iParams[1] = 122;

        int newAnimfile = 0;        // the count starts from 0, not 1 (so 1 -> anim2)
        for (int i = 1; i < 5; ++i)
        {
            if (iParams[i] != -1)
            {
                newAnimfile = i;
                break;
            }
        }
        if (newAnimfile == 0)
            continue;

        std::pair<int,BodyConvDefEntry> k (iParams[0], BodyConvDefEntry(iParams[newAnimfile], newAnimfile));
        m_bodyConvDef.insert(k);
    }

    fileStream.close();
    return true;
}


unsigned int UOAnimMUL::getBodyLookupIndex(int body, int action, int direction, int animFileNumber)
{
    // Stygian Abyss stuff is stored in animationframe*.uop.
    // Every animation before Stygian Abyss is stored in anim mul/idx.

    unsigned index = 0;
    switch (animFileNumber)
    {
        default:
        case 0:
            if (body < 200)
                index = body * 110;
                // 200 * 110 = 22000 animation frame slots for hi detail animations
            else if (body < 400)
                index = 22000 + ((body - 200) * 65);
                // 200 (which is 400 - 200) * 65 = 13000 slots for low detail anims
            else
                index = 35000 + ((body - 400) * 175);
                // 35000 -> 22000 + 13000. Skip this much slots for equip anims
            break;

        case 1:
            if (body < 200)
                index = body * 110;
            else
                index = 22000 + ((body - 200) * 65);
            break;

        case 2:
            if (body < 300)
                index = body * 65;
            else if (body < 400)
                index = 33000 + ((body - 300) * 110);
            else
                index = 35000 + ((body - 400) * 175);
            break;

        case 3:
            if (body < 200)
                index = body * 110;
            else if (body < 400)
                index = 22000 + ((body - 200) * 65);
            else
                index = 35000 + ((body - 400) * 175);
            break;

        case 4:
            if ((body < 200) && (body != 34)) // looks strange, though it works.
                index = body * 110;
            else if (body < 400)
                index = 22000 + ((body - 200) * 65);
            else
                index = 35000 + ((body - 400) * 175);
            break;
    }

    index += action * 5;

    if (direction <= 4)
        index += direction;
    else
        index += direction - (direction - 4) * 2;

    return index;
}


/////

QImage* UOAnimMUL::drawAnimFrame(int bodyID, int action, int direction, int frame, unsigned int hueIndex)
{
    int animFileNumber = 0;

    auto bodyConvDef_it = m_bodyConvDef.find(bodyID);
    if (bodyConvDef_it != m_bodyConvDef.end())  // key found
    {
        bodyID = bodyConvDef_it->second.newID;
        animFileNumber = bodyConvDef_it->second.newFileNum;
    }
    else
    {
        auto bodyDef_it = m_bodyDef.find(bodyID);
        if (bodyDef_it != m_bodyDef.end())      // key found
        {
            bodyID = bodyDef_it->second.newID;
            if (hueIndex == 0)
                hueIndex = bodyDef_it->second.newHue;
        }
    }

    std::string animFileStr;
    switch (animFileNumber)
    {
        case 1:     animFileStr = "anim2";      break;
        case 2:     animFileStr = "anim3";      break;
        case 3:     animFileStr = "anim4";      break;
        case 4:     animFileStr = "anim5";      break;
        default:    animFileStr = "anim";       break;
    }

    unsigned bodyIndex = getBodyLookupIndex(bodyID, action, direction, animFileNumber);

    UOIdx::Entry idxEntry;
    if (! UOIdx::getLookup(m_clientPath + animFileStr + ".idx", bodyIndex, &idxEntry))
    {
        LOG( QString("Error looking up %1.idx (requested id %2).").arg(animFileStr.c_str(), QString::number(bodyID)).toStdString() );
        return nullptr;
    }

    std::ifstream fs_anim;
    fs_anim.open(m_clientPath + animFileStr + ".mul", std::ifstream::in | std::ifstream::binary);
    if (!fs_anim.is_open())
        return nullptr;
    fs_anim.seekg(idxEntry.lookup, std::ios_base::beg);


    /*
    AnimationGroup
        WORD[256] Palette
        DWORD FrameCount
        DWORD[FrameCount] FrameOffset
        Frames

    Seek from the end of Palette plus FrameOffset[FrameNum] bytes to find the start of Frame of interest

    Frame
        WORD xCenter
        WORD yCenter
        WORD Width
        WORD Height
        Chunks

    Chunk
        DWORD header;
        BYTE[xRun] palettePixels;

    See below for the data contained in the header (xOffset, yOffset and xRun).
    If the current chunk header is 0x7FFF7FFF, the image is completed.
    */

    uint16_t palette[256];
    fs_anim.read(reinterpret_cast<char*>(palette), 2 * 256);

    uint32_t frame_count = 0;
    fs_anim.read(reinterpret_cast<char*>(&frame_count), 4);
    if (frame > (int)frame_count)
    {
        fs_anim.close();
        return nullptr;
    }

    uint32_t* frame_offsets = new uint32_t[frame_count];
    fs_anim.read(reinterpret_cast<char*>(frame_offsets), 4 * frame_count);
    fs_anim.seekg(idxEntry.lookup + (2*256) + frame_offsets[frame], std::ios_base::beg);    // go to the selected frame

    int16_t xCenter = 0, yCenter = 0;
    uint16_t width = 0, height = 0;
    fs_anim.read(reinterpret_cast<char*>(&xCenter), 2);
    fs_anim.read(reinterpret_cast<char*>(&yCenter), 2);
    fs_anim.read(reinterpret_cast<char*>(&width),   2);
    fs_anim.read(reinterpret_cast<char*>(&height),  2);

    if (height == 0 || width == 0)
    {
        fs_anim.close();
        return nullptr;
    }

    QImage* img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
    img->fill(0);

    bool applyToGrayOnly = false;   //(hue_index & 0x8000) != 0;

    while ( !fs_anim.eof() && !fs_anim.fail() )
    {
        /*
        HEADER:

        1F	1E	1D	1C	1B	1A	19	18	17	16 | 15	14	13	12	11	10	0F	0E	0D	0C | 0B	0A	09	08	07	06	05	04	03	02	01	00
                xOffset (10 bytes)             |          yOffset (10 bytes)           |            xRun (12 bytes)
        The documentation(*) is wrong, because it inverted the position of x and y offsets. The one above is correct.
        *http://wpdev.sourceforge.net/docs/formats/csharp/animations.html

        xOffset and yOffset are relative to xCenter and yCenter.
        xOffset and yOffset are signed: see the compensation in the code below.
        xRun indicates how many pixels are contained in this line.

        For this piece of code, the MulPatcher source helped A LOT!
        */
        uint32_t header = 0;
        fs_anim.read(reinterpret_cast<char*>(&header), 4);
        if ( header == 0x7FFF7FFF )
            break;

        uint32_t xRun = header & 0xFFF;             // take first 12 bytes
        int32_t xOffset = (header >> 22) & 0x3FF;   // take 10 bytes
        int32_t yOffset = (header >> 12) & 0x3FF;   // take 10 bytes
        // xOffset and yOffset are signed, so we need to compensate for that
        if (xOffset & 512)                  // 512 = 0x200
            xOffset |= (0xFFFFFFFF - 511);  // 511 = 0x1FF
        if (yOffset & 512)
            yOffset |= (0xFFFFFFFF - 511);

        int X = xOffset + xCenter;
        int Y = yOffset + yCenter + height;

        if (X < 0 || Y < 0 || Y >= (int)height || X >= (int)width)
            continue;

        for ( unsigned k = 0; k < xRun; ++k )
        {
            uint8_t palette_index = 0;
            fs_anim.read(reinterpret_cast<char*>(&palette_index), 1);
            ARGB16 color_argb16 = palette[palette_index]; // ^ 0x8000;
            if (hueIndex > 0) // client starts to count from 1 (0 means do not change the color)
            {
                const UOHueEntry& hue = m_UOHues->getHueEntry(hueIndex-1);
                color_argb16 = hue.applyToColor16(color_argb16, applyToGrayOnly);
            }
            ARGB32 color_argb32 = convert_ARGB16_to_ARGB32(color_argb16);

            img->setPixel(X + k, Y, color_argb32.getVal());
        }
    }

    fs_anim.close();
    delete[] frame_offsets;

    return img;
}


}
