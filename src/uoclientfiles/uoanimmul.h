#ifndef UOANIMMUL_H
#define UOANIMMUL_H

#include <string>
#include <map>


class QImage;


namespace uocf
{

class UOHues;


class UOAnimMUL
{
     struct BodyDefEntry
    {
        int newID = 0;
        int newHue = 0;
        BodyDefEntry(int NewID, int NewHue)
        {
            newID = NewID;
            newHue = NewHue;
        }
    };
    struct BodyConvDefEntry
    {
        int newID = 0;
        int newFileNum = 0;
        BodyConvDefEntry(int NewID, int NewFileNum)
        {
            newID = NewID;
            newFileNum = NewFileNum;
        }
    };
public:
    UOHues* m_UOHues;
private:
    std::string m_clientPath;

    std::map<int,BodyDefEntry>       m_bodyDef;       // lookup key (int): oldID
    std::map<int,BodyConvDefEntry>   m_bodyConvDef;   // lookup key (int): oldID

    bool loadBodyDef();
    bool loadBodyConvDef();

    unsigned int getBodyLookupIndex(int body, int action, int direction, int animFileNumber);   // return the in-file index of the anim we want to show

public:
    UOAnimMUL(std::string clientPath);
    QImage* drawAnimFrame(int bodyID, int action, int direction, int frame, int hueIndex);
};


}

#endif // UOANIMMUL_H
