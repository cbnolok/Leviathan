#ifndef UOART_H
#define UOART_H

#include "../cpputils.h"
#include "uoidx.h"
#include <string>

class QImage;

class UOArt
{  
public:
    UOArt(std::string clientPath);
    //~UOArt();

    static const int kLandtilesOffset   = 0;
    static const int kItemsOffset       = 0x4000;   // the (world)items are stored after this id

    QImage* drawArt(unsigned int id, unsigned int hueIndex, bool partialHue);
    
private:
    std::string m_clientPath;
};


#endif // UOART_H
