#include "uoanim.h"


UOAnim::UOAnim(std::string clientPath, std::function<void(int)> reportProgress) :
    m_UOAnimMul(clientPath), m_UOAnimUOP(clientPath, reportProgress)
{
}

QImage* UOAnim::drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex)
{
    if (m_UOAnimUOP.animExists(bodyID))
        return m_UOAnimUOP.drawAnimFrame(bodyID, action, direction, frame, hueIndex);
    else
        return m_UOAnimMul.drawAnimFrame(bodyID, action, direction, frame, hueIndex);
}

