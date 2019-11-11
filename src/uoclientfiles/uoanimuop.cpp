#include "uoanimuop.h"

#include <cstring> // for memcpy
#include <QImage>
#include <QGraphicsPixmapItem>

#include "../cpputils/strings.h"
#include "../cpputils/sysio.h"
#include "uoppackage/uophash.h"
#include "uoppackage/uoppackage.h"
#include "uohues.h"

#include "../globals.h"
#define LOG(x) appendToLog(x)

namespace uocf
{

UOAnimUOP::UOAnimUOP(const std::string &clientPath, std::function<void(int)> reportProgress) :
    m_UOHues(nullptr), m_clientPath(clientPath), m_animationsMatrix{{}}, m_isInitializing(false)
{
    buildAnimTable(reportProgress);
}

// The destructor of the class needs to not be inlined. It calls the destructor of unique_ptr, which needs the type to be complete,
//  so it needs the header which is included here in the cpp, while in the h the class was only forward declared.
UOAnimUOP::~UOAnimUOP() = default;

void UOAnimUOP::buildAnimTable(const std::function<void(int)>& reportProgress)
{
    m_isInitializing = true;
    //memset((void*)m_animationsMatrix, 0, sizeof(m_animationsMatrix[0][0]) * kAnimIdMax * kGroupIdMax);

    // We need to know which animations are in the uop files

    LOG("Building UOP animations table...");
    int progressVal = 0;

    // Parse anim data in each AnimationFrame*.uop
    const int uopFileCount = 4;
    for (int uopFile_i = 1; uopFile_i <= uopFileCount; ++uopFile_i)
    {
        std::string path = m_clientPath + "AnimationFrame" + std::to_string(uopFile_i) + ".uop";

        if (!isValidFile(path))
        {
            LOG("File does not exist: " + path);
            continue;
        }

        // Read the data in order to access later to each file by its hash
        m_animUOPs[uopFile_i - 1] = std::make_unique<uopp::UOPPackage>();
        uopp::UOPPackage* package = m_animUOPs[uopFile_i - 1].get();
        uopp::UOPError uopErrorQueue;
        package->load(path, &uopErrorQueue);
        if (uopErrorQueue.errorOccurred())   // check if there was an error when extracting the uop file
        {
            LOG("UOPPackage error!");
            LOG(uopErrorQueue.buildErrorsString());
            return;
        }

        for (unsigned block_i = 0, block_max = package->getBlocksCount(); block_i < block_max; ++block_i)
        {
            const uopp::UOPBlock* curBlock = package->getBlock(block_i);
            for (unsigned file_i = 0, file_max = curBlock->getFilesCount(); file_i < file_max; ++file_i)
            {
                const uopp::UOPFile* curFile = curBlock->getFile(file_i);
                UOPAnimationData data = {};
                data.animFileIdx = uopFile_i;
                data.blockIdx = block_i;
                data.fileIdx = file_i;
                data.hash = curFile->getFileHash();
                m_animationsData.emplace_back(std::move(data));
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
    //  so this way we assign work to the OpenMP threads (in its thread pool) less often (dunno if it actually causes overhead, but who knows...)
    for (int groupId = 0; groupId < kGroupIdMax; ++groupId)
    {
        #pragma omp parallel for schedule(static)   // split the workload between some threads with OpenMP!
        for (int animId = 0; animId < kAnimIdMax; ++animId)
        {
            char hashString[100];
            snprintf(hashString, sizeof(hashString), "build/animationlegacyframe/%06i/%02i.bin", animId, groupId);
            unsigned long long hash = uopp::hashFileName(hashString);
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

        if (reportProgress)
        {
            int progressValNow = ( (groupId*100)/kGroupIdMax );
            if ( progressValNow > progressVal )
            {
                progressVal = progressValNow;
                reportProgress(progressVal);
            }
        }

    }
    m_isInitializing = false;
}

bool UOAnimUOP::animExists(int animID)
{
    if (isInitializing())
        return false;

    for (int i = 0; i < kGroupIdMax; ++i)
    {
        if (m_animationsMatrix[animID][i] != nullptr)   // do we have almost an action (group) for this animId?
            return true;
    }
    return false;
}

UOAnimUOP::UOPFrameData UOAnimUOP::loadFrameData(int animID, int groupID, int direction, int frame, std::vector<char>* decompressedData)
{
    if (isInitializing())
        return UOPFrameData{};

    UOPAnimationData* animData = m_animationsMatrix[animID][groupID];

    // extract selected frame data from the UOP in memory
    std::unique_ptr<uopp::UOPPackage>& animPkg = m_animUOPs[animData->animFileIdx - 1];
    uopp::UOPFile* animFile = animPkg->getFileByIndex(animData->blockIdx, animData->fileIdx);

    unsigned int decDataSize = animFile->getDecompressedSize();
    decompressedData->resize(decDataSize);


    std::shared_ptr<char[]> decompressedDataRaw;

    uopp::UOPError uopErrorQueue;
    std::ifstream fin = animPkg->getOpenedStream();
    animFile->readPackedData(fin, &uopErrorQueue);
    fin.close();
    animFile->unpack(&decompressedDataRaw, &uopErrorQueue);
    animFile->freePackedData();

    decompressedData->resize(animFile->getDataSize());
    memcpy(decompressedData->data(), decompressedDataRaw.get(), animFile->getDataSize());

    if (uopErrorQueue.errorOccurred())   // check if there was an error when extracting the uop file
    {
        LOG("UOPPackage error!");
        LOG(uopErrorQueue.buildErrorsString());
        return UOPFrameData{};
    }

    const char* decData = decompressedData->data();
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
    uint frameCount = 0;
    memcpy(&frameCount, decData + decDataOff, 4);
    decDataOff += 4;
    //address of the first frame
    uint frameAddress = 0;
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
        frameDataVec.emplace_back(std::move(curFrameData));
    }
    int vectorSize = (int)frameDataVec.size();
    if (vectorSize < 50)
    {
        while (vectorSize != 50)
        {
            frameDataVec.emplace_back(UOPFrameData{ });
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


QImage* UOAnimUOP::drawAnimFrame(int bodyID, int action, int direction, int frame, unsigned int hueIndex)
{
    if (isInitializing())
        return nullptr;

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
    std::vector<char> decompressedData;    // buffer filled by loadFrameData, it will contain the decompressed animation data
    UOPFrameData frameData = loadFrameData(bodyID, groupID, direction, frame, &decompressedData);
    if (frameData.pixelDataOffset == 0) // uninitialized --> error
        return nullptr;

    const char* decData = decompressedData.data();
    size_t decDataOff = frameData.dataStart + frameData.pixelDataOffset;
    size_t decDataSize = decompressedData.size();     // size of the decompressed data (in bytes)

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
        // For the header structure read inside UOAnimMUL.cpp
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
            if (hueIndex > 0)   // client starts to count from 1 (0 means do not change the color)
            {
                const UOHueEntry& hue = m_UOHues->getHueEntry(hueIndex-1);
                color_argb16 = hue.applyToColor16(color_argb16, applyToGrayOnly);
            }
            ARGB32 color_argb32 = convert_ARGB16_to_ARGB32(color_argb16);

            img->setPixel(X + k, Y, color_argb32.getVal());
        }
    }

    return img;
}


}
