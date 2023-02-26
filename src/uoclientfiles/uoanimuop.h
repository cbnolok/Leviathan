#ifndef UOANIMUOP_H
#define UOANIMUOP_H

#include <string>
#include <vector>
#include <functional>   // for std::function (callback)
#include <memory>


class QImage;

namespace uopp {
    class UOPPackage;
}

namespace uocf
{

class UOHues;


class UOAnimUOP
{
    struct UOPAnimationData
    {
        int animFileIdx;        // AnimationFrame(x).uop
        unsigned int blockIdx;
        unsigned int fileIdx;
        unsigned long long hash;
    };

    struct UOPFrameData
    {
        size_t dataStart = 0;
        //uint16_t frameId;
        unsigned int pixelDataOffset = 0;
    };

public:
    UOAnimUOP(const std::string& clientPath, std::function<void (int)> reportProgress);
    ~UOAnimUOP();

    bool isInitializing() const {
        return m_isInitializing;
    }
    bool animExists(int animID);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, unsigned int hueIndex);

    UOHues* m_UOHues;
private:
    std::string m_clientPath;
    std::unique_ptr<uopp::UOPPackage> m_animUOPs[4];
    std::vector<UOPAnimationData> m_animationsData;

    // sort animationsData by [animID][groupID]:
    static const int kAnimIdMax = 2048;   // animation ID
    static const int kGroupIdMax = 100;   // action ID
    UOPAnimationData* m_animationsMatrix[kAnimIdMax][kGroupIdMax];  // the matrix is thread-safe if we aren't writing in the same position in different threads
    bool m_isInitializing;

    void buildAnimTable(const std::function<void(int)>& reportProgress);
    UOPFrameData loadFrameData(int animID, int groupID, int direction, int frame, std::vector<char>* decompressedData);
};


}

#endif // UOANIMUOP_H
