#include "uoanim.h"

namespace uocf
{

UOAnim::UOAnim(std::string clientPath, std::function<void(int)> reportProgress) :
    m_UOAnimMUL(clientPath), m_UOAnimUOP(clientPath, reportProgress)
{
}

void UOAnim::setHuesCachePointer(UOHues* hues)
{
    m_UOAnimMUL.m_UOHues = hues;
    m_UOAnimUOP.m_UOHues = hues;
}

QImage* UOAnim::drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex)
{
    if (m_UOAnimUOP.animExists(bodyID))
        return m_UOAnimUOP.drawAnimFrame(bodyID, action, direction, frame, hueIndex);
    else
        return m_UOAnimMUL.drawAnimFrame(bodyID, action, direction, frame, hueIndex);
}


}
