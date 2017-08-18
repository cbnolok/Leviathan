#ifndef UOART_H
#define UOART_H

#include "../common.h"
#include "uoidx.h"
#include <string>

class QImage;

class UOArt
{  
public:
    UOArt(std::string clientPath);
    //~UOArt();
    QImage* drawArt(int id, int hue_index, bool partialHue);
    
private:
    std::string m_clientPath;
};


#endif // UOART_H
