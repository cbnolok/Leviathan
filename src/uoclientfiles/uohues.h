#ifndef UOHUES_H
#define UOHUES_H

#include <cctype>
#include <string>


namespace uocf
{

//--------Utilities for easy handling of 16 bpp and 32 bpp RGB colors

class ARGB16;
class ARGB32;
ARGB32 convert_ARGB16_to_ARGB32(ARGB16 argb16, bool maxOpacity = true);
ARGB16 convert_ARGB32_to_ARGB16(ARGB32 argb32, bool maxOpacity = true);


class ARGB16    // Colors used by the client are RGB16
{
private:
    uint16_t m_color;   // arrr rrgg gggb bbbb

public:
    ARGB16() {
        m_color = 0;
    }

    uint8_t getA() const {
        return (m_color & 0x8000) >> 15;
    }
    uint8_t getR() const {
        return (m_color & 0xF800) >> 10;
    }
    uint8_t getG() const {
        return (m_color & 0x07E0) >> 5;
    }
    uint8_t getB() const {
        return (m_color & 0x001F);
    }
    uint16_t getVal() const {
        return m_color;
    }

    void setA(unsigned int a) {
        m_color = (this->getB()) | (this->getG() << 5) | (this->getR() << 10) | (a & 0x1F << 15);
    }
    void setR(unsigned int r) {
        m_color = (this->getB()) | (this->getG() << 5) | ((r & 0x1F) << 10) | (this->getA() << 15);
    }
    void setG(unsigned int g) {
        m_color = (this->getB()) | ((g & 0x1F) << 5) | (this->getR() << 10) | (this->getA() << 15);
    }
    void setB(unsigned int b) {
        m_color = (b & 0x1F) | (this->getG() << 5) | (this->getR() << 10) | (this->getA() << 15);
    }
    void setVal(uint16_t val) {
        m_color = val;
    }

    ARGB16(uint16_t color16) {
        this->setVal(color16);
    }
    ARGB16(unsigned int a, unsigned int r, unsigned int g, unsigned int b) {
        m_color = 0;
        this->setA(a);
        this->setR(r);
        this->setG(g);
        this->setB(b);
    }
    ARGB16& operator= (uint16_t color16) {
        this->setVal(color16);
        return *this;
    }
    template<typename castT> operator castT() const {
        if (sizeof(castT) < sizeof(m_color))    // casting to a data type too small?
            return 0;
        return this->getVal();
    }
};

class ARGB32    // Colors used by Leviathan's interface are RGB32
{
private:
    uint8_t m_color_a, m_color_r, m_color_g, m_color_b;

public:
    ARGB32() {
        m_color_a = m_color_r = m_color_g = m_color_b = 0;
    }

    uint8_t getA() const {
        return m_color_a;
    }
    uint8_t getR() const {
        return m_color_r;
    }
    uint8_t getG() const {
        return m_color_g;
    }
    uint8_t getB() const {
        return m_color_b;
    }
    uint32_t getVal() const {
        return ( (this->getA() << 24) | (this->getR() << 16) | (this->getG() << 8) | this->getB() );
    }

    void setA(uint8_t a) {
        m_color_a = a;
    }
    void setR(uint8_t r) {
        m_color_r = r;
    }
    void setG(uint8_t g) {
        m_color_g = g;
    }
    void setB(uint8_t b) {
        m_color_b = b;
    }
    void setVal(uint32_t val) {
        m_color_a = (   (val & 0xFF000000) >> 24    );
        m_color_r = (   (val & 0x00FF0000) >> 16    );
        m_color_g = (   (val & 0x0000FF00) >> 8     );
        m_color_b =     (val & 0x000000FF);
    }

    ARGB32(uint32_t color32) {
        m_color_a = m_color_r = m_color_g = m_color_b = 0;
        this->setVal(color32);
    }
    ARGB32(ARGB16 argb16) {
        this->setVal(convert_ARGB16_to_ARGB32(argb16));
    }
    ARGB32(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
        this->setA(a);
        this->setR(r);
        this->setG(g);
        this->setB(b);
    }
    ARGB32& operator= (uint32_t color32) {
        this->setVal(color32);
        return *this;
    }
    ARGB32& operator= (ARGB16 argb16) {
         this->setVal(convert_ARGB16_to_ARGB32(argb16));
        return *this;
    }
    template<typename castT> operator castT() const {
        if (sizeof(castT) < sizeof(uint32_t))    // casting to a data type too small?
            return 0;
        return this->getVal();
    }

    void adjustBrightness(int percent);
};


//--------hues.mul

class UOHues;
struct UOHueEntry
{
    friend class UOHues;
    static const int kColorTableSize = 32;

private:
    //uint32_t index;
    char name[20];
    uint16_t color_table[kColorTableSize];

public:
    ARGB16 getColor(unsigned int index) const;
    std::string getName() const;

    ARGB16 applyToColor(ARGB16 color16, bool applyToGrayOnly = false);    // apply hue to the RGB16 color
};


class UOHues
{
public:
    UOHues(const std::string &huesPath);  // loads hues
    //~UOHues();
    UOHueEntry getHueEntry(int index) const;

private:
    UOHueEntry hues[3000];
};


}

#endif // UOHUES_H
