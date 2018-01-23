#include "uoanimuop.h"
#include "../globals.h"
#include "../cpputils.h"
#include "../cpputils_sysio.h"
#include "uoppackage/UOPError.h"
#include "uoppackage/UOPPackage.h"
#include "uoppackage/UOPBlock.h"
#include "uoppackage/UOPFile.h"
#include "uohues.h"
#include <QImage>
#include <QGraphicsPixmapItem>


UOAnimUOP::UOAnimUOP(std::string clientPath, std::function<void(int)> reportProgress) :
    m_clientPath(clientPath), m_isLoading(false)
{
    buildAnimTable(reportProgress);
}

//UOAnimUOP::~UOAnimUOP()
//{
//}

void UOAnimUOP::buildAnimTable(std::function<void(int)> reportProgress)
{
    m_isLoading = true;
    memset((void*)m_animationsMatrix, 0, sizeof(m_animationsMatrix[0][0]) * kAnimIdMax * kGroupIdMax);

    // We need to know which animations are in the uop files

    appendToLog("Building UOP animations table...");
    int progressVal = 0;

    // Parse anim data in each AnimationFrame*.uop
    const int uopFileCount = 4;
    for (int uopFile_i = 1; uopFile_i <= uopFileCount; ++uopFile_i)
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

            // it's so fast we can even do not report progress (or use OpenMP)...
            /*
            int progressValNow = ( ((block_i*100)/(block_max-1)) / (uopFileCount - uopFile_i + 1) );
            if ( reportProgress && (progressValNow > progressVal) )
            {
                progressVal = progressValNow;
                reportProgress(progressVal);    // it's a bit of a mess to pass the progress to the main Qt thread...
            }
            */
        }
    }

    // Find the groups for each animation we have found
    //  (groups are different actions but they do not have sequential numbers)

    progressVal = 0;

    // It would be more logical to have animId as outer loop and groupId as inner loop, but the kGroupIdMax is < than kAnimIdMax,
    //  so this way we create the threads much less often.
    for (int groupId = 0; groupId < kGroupIdMax; ++groupId)
    {
        #pragma omp parallel for schedule(static)   // split the workload between some threads with OpenMP!
        for (int animId = 0; animId < kAnimIdMax; ++animId)
        {
            char hashString[100];
            sprintf(hashString, "build/animationlegacyframe/%06i/%02i.bin", animId, groupId);
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
               m_animationsMatrix[animId][groupId] = &m_animationsData[found];
        }

        int progressValNow = ( (groupId*100)/kGroupIdMax );
        if ( reportProgress && (progressValNow > progressVal) )
        {
            progressVal = progressValNow;
            reportProgress(progressVal);    // it's a bit of a mess to pass the progress to the main Qt thread...
        }

    }
    m_isLoading = false;
}

bool UOAnimUOP::animExists(int animID)
{
    for (int i = 0; i < kGroupIdMax; ++i)
    {
        if (m_animationsMatrix[animID][i] != nullptr)   // do we have almost an action (group) for this animId?
            return true;
    }
    return false;
}

UOAnimUOP::UOPFrameData UOAnimUOP::loadFrameData(int animID, int groupID, int direction, int frame, char* &decData, size_t &decDataSize)
{
    UOPAnimationData* animData = m_animationsMatrix[animID][groupID];

    uoppackage::UOPPackage& animPkg = m_animUOPs[animData->animFileIdx - 1];
    uoppackage::UOPFile* animFile = animPkg.getFileByIndex((int)animData->blockIdx, (int)animData->fileIdx);
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

    //unsigned dirFrameCount = pixelDataOffsets.size() / 5;   // 5 = number of directions
    //unsigned dirFrameStartIdx = dirFrameCount * direction;
    unsigned dirFrameStartIdx = (frameCount / 5) * direction;

    // select the frame i want to display
    UOPFrameData &frameData = frameDataVec[frame + dirFrameStartIdx];
    return frameData;
}


QImage* UOAnimUOP::drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex)
{
    // select the group (action) we have chosen or, if invalid, the first valid one
    int groupID = -1;
    if (m_animationsMatrix[bodyID][action] != nullptr)
        groupID = action;
    else
    {
        for (int i = 0; i < kGroupIdMax; ++i)
        {
            if (m_animationsMatrix[bodyID][i] != nullptr)
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

    int16_t xCenter = 0, yCenter = 0;
    int16_t width = 0, height = 0;

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

    bool applyToGrayOnly = false;   //(hueIndex & 0x8000) != 0;

    while ( decDataOff < decDataSize )
    {
        // For the header structure read inside uoanimmul.cpp
        uint32_t header = 0;
        memcpy(&header, decData + decDataOff, 4);
        decDataOff += 4;
        if ( header == 0x7FFF7FFF )
            break;

        uint32_t xRun = header & 0xFFF;              // take first 12 bytes
        uint32_t xOffset = (header >> 22) & 0x3FF;   // take 10 bytes
        uint32_t yOffset = (header >> 12) & 0x3FF;   // take 10 bytes
        // xOffset and yOffset are signed, so we need to compensate for that
        if (xOffset & 512)                  // 512 = 0x200
            xOffset |= (0xFFFFFFFF - 511);  // 511 = 0x1FF
        if (yOffset & 512)
            yOffset |= (0xFFFFFFFF - 511);

        int X = xOffset + xCenter;
        int Y = yOffset + yCenter + height;

        if (X < 0 || Y < 0 || Y >= height || X >= width)
            continue;

        for ( unsigned k = 0; k < xRun; ++k )
        {
            uint8_t palette_index = 0;
            memcpy(&palette_index, decData + decDataOff, 1);
            decDataOff += 1;

            ARGB16 color_argb16 = palette[palette_index]; // ^ 0x8000;
            if (hueIndex != 0)
            {
                UOHueEntry hue = g_UOHues->getHueEntry(hueIndex);
                color_argb16 = hue.applyToColor(color_argb16, applyToGrayOnly);
            }
            ARGB32 color_argb32 = argb16_to_argb32(color_argb16);

            img->setPixel(X + k, Y, color_argb32.getVal());
        }
    }

    delete[] decData;
    return img;
}

