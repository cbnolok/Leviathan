#include "uoanimuop.h"
#include "../globals.h"
#include "../common.h"
#include "../sysio.h"
#include "uoppackage/UOPError.h"
#include "uoppackage/UOPPackage.h"
#include "uoppackage/UOPBlock.h"
#include "uoppackage/UOPFile.h"
#include "uohues.h"
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QApplication>


UOAnimUOP::UOAnimUOP(std::string clientPath) :
    m_clientPath(clientPath)
{
    buildAnimTable();
}

//UOAnimUOP::~UOAnimUOP()
//{
//}

void UOAnimUOP::buildAnimTable()
{
    // We need to know which animations are in the uop files

    appendToLog("Building UOP animations table...");
    /*
    emit notifyTPMessage("Loading client files...\nParsing AnimationFrame UOP files...");
    emit notifyTPProgressVal(0);
    emit notifyTPProgressMax(0);
    */
    unsigned progressVal = 0;

    // Parse anim data in each AnimationFrame*.uop
    for (int uopFile_i = 1; uopFile_i <= 4; ++uopFile_i)
    {
        std::string path = m_clientPath + "AnimationFrame" + std::to_string(uopFile_i) + ".uop";

        if (!isValidFile(path))
        {
            appendToLog("File does not exist: " + path);
            continue;
        }

        // Read the data in order to access later to each file by its hash
        uoppackage::UOPPackage& package = m_animUOPs[uopFile_i - 1];
        package.load(path);
        if (uoppackage::errorHandler.errorOccurred())   // check if there was an error when extracting the uop file
        {
            appendToLog("UOPPackage error!");
            auto errors = uoppackage::errorHandler.getErrorQueue();
            for (std::string str : errors)
                appendToLog(str);
            return;
        }

        auto blocks = package.getBlocks();

        for (size_t block_i = 0, block_max = blocks.size(); block_i < block_max; ++block_i)
        {
            uoppackage::UOPBlock* curBlock = blocks[block_i];
            auto files = curBlock->getFiles();
            for (size_t file_i = 0, file_max = files.size(); file_i < file_max; ++file_i)
            {
                uoppackage::UOPFile* curFile = files[file_i];

                UOPAnimationData data;
                data.animFileIdx = uopFile_i;
                data.blockIdx = block_i;
                data.fileIdx = file_i;
                data.hash = curFile->getFileHash();
                m_animationsData.push_back(data);
            }

            unsigned progressValNow = ( (block_i*100)/block_max );
            if (progressValNow > progressVal)
            {
                progressVal = progressValNow;
                QCoreApplication::processEvents();  // Update the UI to see the progress bar moving
            }
        }
    }

    // Find the groups for each animation we have found
    //  (groups are different actions but they do not have sequential numbers)

    /*
    emit notifyTPMessage("Loading client files...\nBuilding UOP animations table...");
    emit notifyTPProgressMax(150);
    */
    progressVal = 0;

    for (int animId = 0; animId < 2048; ++animId)
    {
        for (int grpId = 0; grpId < 100; ++grpId)
        {
            char hashString[100];
            sprintf(hashString, "build/animationlegacyframe/%06i/%02i.bin", animId, grpId);
            unsigned long long hash = uoppackage::UOPPackage::getHash(hashString);
            int found = -1;
            for (int i = 0, max = (int)m_animationsData.size(); i < max; ++i)
            {
                if (m_animationsData[i].hash == hash)
                {
                    found = i;
                    break;
                }
            }
            if (found != -1)
               m_animationsMatrix[animId][grpId] = &m_animationsData[found];
        }

        unsigned progressValNow = (unsigned)( (animId*150)/2048 );
        if (progressValNow > progressVal)
        {
            progressVal = progressValNow;
            //emit notifyTPProgressVal((int)progressVal);
            QCoreApplication::processEvents();  // Process received events to avoid the GUI freezing.
        }
    }

}

bool UOAnimUOP::animExists(int animID)
{
    if (m_animationsMatrix.find(animID) != m_animationsMatrix.end())
        return true;
    return false;
}

int UOAnimUOP::getLookupAnimID(int body)
{
    // List of animation indexes as of 7.0.59.4
    /*
    const int notInARange[] = {
        130, 197, 198, 267, 270, 286, 287, 293, 428, 475,
        483, 485, 499, 509, 534, 541, 547, 609, 610, 632,
        647, 650, 652, 705, 707, 708, 753, 769, 795, 826,
        843, 990, 1279, 1281, 1308, 1309
    };
    */
    // ranges: 320-344, 585-604, 654-678, 690-693, 696-699, 713-743,
    //  829-832, 1026-1071, 1244-1248, 1251-1255, 1285-1294, 1400-1433
    return body;
}

UOAnimUOP::UOPFrameData UOAnimUOP::loadFrameData(int animID, int groupID, int direction, int frame, char* &decData, size_t &decDataSize)
{
    UOPAnimationData* animData = m_animationsMatrix[animID][groupID];

    uoppackage::UOPFile* animFile = m_animUOPs[animData->animFileIdx - 1].getFileByIndex(animData->blockIdx, animData->fileIdx);
    std::string path = m_clientPath + "AnimationFrame" + std::to_string(animData->animFileIdx) + ".uop";
    std::ifstream fin;

    // extract selected frame data from the UOP in memory
    fin.open(path, std::ifstream::in | std::ifstream::binary );
    animFile->unpack(fin, decData, decDataSize);
    fin.close();

    if (uoppackage::errorHandler.errorOccurred())   // check if there was an error when extracting the uop file
    {
        appendToLog("UOPPackage error!");
        auto errors = uoppackage::errorHandler.getErrorQueue();
        for (std::string str : errors)
            appendToLog(str);
        return UOPFrameData{};
    }

    size_t decDataOff = 0;

    // read frame header

    //format id?
    decDataOff += 4;
    //version
    decDataOff += 4;
    //decompressed data size
    decDataOff += 4;
    //anim id
    decDataOff += 4;
    //8 bytes unknown [1]
    // File time (number of 100-nanosecond intervals since January 1, 1601 UTC)?
    decDataOff += 8;
    //unknown [2]
    decDataOff += 2;
    //unknown [3]
    decDataOff += 2;
    //header length
    decDataOff += 4;
    //framecount (total frame number, for every direction)
    uint32_t frameCount;
    memcpy(&frameCount, decData + decDataOff, 4);
    decDataOff += 4;
    //address of the first frame
    uint32_t frameAddress;
    memcpy(&frameAddress, decData + decDataOff, 4);

    decDataOff = frameAddress;


    // read frame data

    //  each frame of an animation (with given animID and groupID) has an header. the headers are stored sequentially
    //  and separatedly from the pixel data.
    std::vector<UOPFrameData> frameDataVec;

    for (unsigned int frame_i = 0; frame_i < frameCount; ++frame_i)
    {
        UOPFrameData curFrameData;
        curFrameData.dataStart = decDataOff;

        //anim group
        decDataOff += 2;
        //frame id
        //memcpy(&curFrameData.frameId, decData + decDataOff, 2);
        decDataOff += 2;
        //8 bytes unknown
        decDataOff += 8;
        //offset (starting from dataStart) to this frame image data
        memcpy(&curFrameData.pixelDataOffset, decData + decDataOff, 4);
        decDataOff += 4;

        /*
        int vsize = frameDataVec.size();
        if (vsize + 1 != curFrameData.frameId)
        {
            while (vsize + 1 != curFrameData.frameId)
            {
                frameDataVec.push_back(UOPFrameData{ });
                ++vsize;
            }
        }
        */
        frameDataVec.push_back(curFrameData);
    }
    int vectorSize = (int)frameDataVec.size();
    if (vectorSize < 50)
    {
        while (vectorSize != 50)
        {
            frameDataVec.push_back(UOPFrameData{ });
            ++vectorSize;
        }
    }

    //unsigned int dirFrameCount = pixelDataOffsets.size() / 5;   // 5 = number of directions
    //unsigned int dirFrameStartIdx = dirFrameCount * direction;
    unsigned int dirFrameStartIdx = (frameCount / 5) * direction;

    // select the frame i want to display
    UOPFrameData &frameData = frameDataVec[frame + dirFrameStartIdx];
    return frameData;
}


QImage* UOAnimUOP::drawAnimFrame(int bodyID, int action, int direction, int frame, int hue_index)
{
    int animID = getLookupAnimID(bodyID);

    // select the group (action) we have chosen or, if invalid, the first valid one
    int groupID = -1;
    if (m_animationsMatrix[animID][action] != nullptr)
        groupID = action;
    else
    {
        for (int i = 0; i < 100; ++i)
        {
            if (m_animationsMatrix[animID][i] != nullptr)
                groupID = i;
        }
        if (groupID == -1)
            return nullptr;
    }

    // get from the UOP file the raw frame data (which has the same encoding as the MUL frame data)
    char* decData = nullptr;    // buffer filled by loadFrameData, it will contain the decompressed animation data
    size_t decDataSize = 0;     // size of the decompressed data (in bytes)
    UOPFrameData frameData = loadFrameData(bodyID, groupID, direction, frame, decData, decDataSize);
    if (frameData.pixelDataOffset == 0) // uninitialized --> error
    {
        delete[] decData;
        return nullptr;
    }

    size_t decDataOff = frameData.dataStart + frameData.pixelDataOffset;

    int16_t palette[256];
    memcpy(&palette, decData + decDataOff, 512);
    decDataOff += 512;

    int_fast16_t xCenter = 0, yCenter = 0;
    int_fast16_t width = 0, height = 0;

    memcpy(&xCenter, decData + decDataOff, 2);
    decDataOff += 2;

    memcpy(&yCenter, decData + decDataOff, 2);
    decDataOff += 2;

    memcpy(&width, decData + decDataOff, 2);
    decDataOff += 2;

    memcpy(&height, decData + decDataOff, 2);
    decDataOff += 2;

    if (height == 0 || width == 0 || height > 800 || width > 800)
        return nullptr;

    QImage* img = new QImage((int)width, (int)height, QImage::Format_ARGB32);
    img->fill(0);

    bool applyToGrayOnly = false;   //(hue_index & 0x8000) != 0;

    while ( decDataOff < decDataSize )
    {
        // For the header structure read inside uoanimmul.cpp
        uint_fast32_t header = 0;
        memcpy(&header, decData + decDataOff, 4);
        decDataOff += 4;
        if ( header == 0x7FFF7FFF )
            break;

        uint_fast32_t xRun = header & 0xFFF;              // take first 12 bytes
        uint_fast32_t xOffset = (header >> 22) & 0x3FF;   // take 10 bytes
        uint_fast32_t yOffset = (header >> 12) & 0x3FF;   // take 10 bytes
        // xOffset and yOffset are signed, so we need to compensate for that
        if (xOffset & 512)                  // 512 = 0x200
            xOffset |= (0xFFFFFFFF - 511);  // 511 = 0x1FF
        if (yOffset & 512)
            yOffset |= (0xFFFFFFFF - 511);

        int X = xOffset + xCenter;
        int Y = yOffset + yCenter + height;

        if (X < 0 || Y < 0 || Y >= height || X >= width)
            continue;

        for ( unsigned int k = 0; k < xRun; ++k )
        {
            uint_fast8_t palette_index = 0;
            memcpy(&palette_index, decData + decDataOff, 1);
            decDataOff += 1;

            ARGB16 color_argb16 = palette[palette_index]; // ^ 0x8000;
            if (hue_index != 0)
            {
                UOHueEntry hue = g_UOHues->getHue(hue_index);
                color_argb16 = hue.applyToColor(color_argb16, applyToGrayOnly);
            }
            ARGB32 color_argb32 = argb16_to_argb32(color_argb16);

            img->setPixel(X + k, Y, color_argb32.getVal());
        }
    }

    delete[] decData;
    return img;
}

