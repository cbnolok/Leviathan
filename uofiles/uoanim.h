#ifndef UOANIM_H
#define UOANIM_H

#include "uoanimmul.h"
#include "uoanimuop.h"
#include <unordered_map>

class QImage;

class UOAnim
{
public:
    UOAnim(std::string clientPath);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex);

private:
    UOAnimMul m_UOAnimMul;
    UOAnimUOP m_UOAnimUOP;
};

#endif // UOANIM_H
