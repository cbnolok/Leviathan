#ifndef UOANIM_H
#define UOANIM_H

#include <functional>   // for std::function (callback)

#include "uoanimmul.h"
#include "uoanimuop.h"


class QImage;

namespace uocf
{


class UOHues;


class UOAnim
{
public:
    UOAnim(const std::string& clientPath, std::function<void(int)> reportProgress = nullptr);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, unsigned int hueIndex);

    void setCachePointers(UOHues* hues);

private:
    UOAnimMUL m_UOAnimMUL;
    UOAnimUOP m_UOAnimUOP;
};


}

#endif // UOANIM_H
