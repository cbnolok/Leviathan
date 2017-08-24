#ifndef UOANIMUOP_H
#define UOANIMUOP_H

#include "uoppackage/UOPPackage.h"
#include <string>
#include <vector>
#include <unordered_map>

//#include <QObject>


class QImage;

class UOAnimUOP //: QObject
{
    /*
    Q_OBJECT
    // Need it to be a QObject to have signals and slots, in order to send progress info to the other thread.

signals:
    // TP: Task Progress (notify the progress status to SubDlg_TaskProgress)
    void notifyTPProgressMax(int max);
    void notifyTPProgressVal(int val);
    void notifyTPMessage(QString msg);
*/
public:
    UOAnimUOP(std::string clientPath);
    //~UOAnimUOP();

    bool animExists(int animID);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, int hue_index);

private:
    struct UOPAnimationData
    {
        int animFileIdx;        // AnimationFrame(x).uop
        size_t blockIdx;
        size_t fileIdx;
        unsigned long long hash;
    };

    struct UOPFrameData
    {
        size_t dataStart;
        //uint16_t frameId;
        unsigned int pixelDataOffset;
    };

    std::string m_clientPath;
    uoppackage::UOPPackage m_animUOPs[4];

    std::vector<UOPAnimationData> m_animationsData;

    // sort animationsData by [groupID][animID]:
    // m_animationsMatrix[groupID] contains a map which [animID] contains the UOPAnimationData*
    std::unordered_map<int, std::unordered_map<int,UOPAnimationData*>> m_animationsMatrix;

    void buildAnimTable();
    int getLookupAnimID(int body);
    UOPFrameData loadFrameData(int animID, int groupID, int direction, int frame, char *&decData, size_t& decDataSize); // decData: buffer for decompressed anim data
};

#endif // UOANIMUOP_H
