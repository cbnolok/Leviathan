#ifndef COLORS_H
#define COLORS_H

#include <cstdint>


namespace uocf
{

//--------Utilities for easy handling of 16 bpp and 32 bpp RGB colors

class ARGB16;
class ARGB32;
ARGB32 convert_ARGB16_to_ARGB32(const ARGB16 argb16, bool maxOpacity = true) noexcept;
ARGB16 convert_ARGB32_to_ARGB16(const ARGB32 argb32, bool maxOpacity = true) noexcept;


class ARGB16    // Colors used by the client are RGB16
{
private:
    unsigned short m_color;   // arrr rrgg gggb bbbb

public:
    ARGB16() noexcept {
        m_color = 0;
    }

    inline unsigned char getA() const noexcept {
        return (m_color >> 15) & 0x1F;
    }
    inline unsigned char getR() const noexcept {
        return (m_color >> 10) & 0x1F;
    }
    inline unsigned char getG() const noexcept {
        return (m_color >> 5) & 0x1F;
    }
    inline unsigned char getB() const noexcept {
        return (m_color & 0x1F);
    }
    inline unsigned short getVal() const noexcept {
        return m_color;
    }

    void setA(unsigned short a) noexcept {
        m_color = static_cast<unsigned short>((this->getB()) | (this->getG() << 5) | (this->getR() << 10) | (a & 0x1F << 15));
    }
    void setR(unsigned short r) noexcept {
        m_color = static_cast<unsigned short>((this->getB()) | (this->getG() << 5) | ((r & 0x1F) << 10) | (this->getA() << 15));
    }
    void setG(unsigned short g) noexcept {
        m_color = static_cast<unsigned short>((this->getB()) | ((g & 0x1F) << 5) | (this->getR() << 10) | (this->getA() << 15));
    }
    void setB(unsigned short b) noexcept {
        m_color = static_cast<unsigned short>((b & 0x1F) | (this->getG() << 5) | (this->getR() << 10) | (this->getA() << 15));
    }
    inline void setVal(unsigned short val) noexcept {
        m_color = val & 0xFFFF;
    }

    ARGB16(unsigned short color16) {
        this->setVal(color16);
    }
    ARGB16(unsigned char a, unsigned char r, unsigned char g, unsigned char b) noexcept {
        m_color = static_cast<unsigned short>((b & 0x1F) | ((g & 0x1F) << 5) | ((r & 0x1F) << 10) | ((a & 0x1F) << 15));
    }
    ARGB16& operator= (unsigned short color16) noexcept {
        this->setVal(color16);
        return *this;
    }
    template<typename castT> operator castT() const noexcept {
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
    ARGB32() noexcept {
        m_color_a = m_color_r = m_color_g = m_color_b = 0;
    }

    inline uint8_t getA() const noexcept {
        return m_color_a;
    }
    inline uint8_t getR() const noexcept {
        return m_color_r;
    }
    inline uint8_t getG() const noexcept {
        return m_color_g;
    }
    inline uint8_t getB() const noexcept {
        return m_color_b;
    }
    unsigned int getVal() const noexcept {
        return static_cast<unsigned int>( (this->getA() << 24) | (this->getR() << 16) | (this->getG() << 8) | this->getB() );
    }

    inline void setA(uint8_t a) noexcept {
        m_color_a = a;
    }
    inline void setR(uint8_t r) noexcept {
        m_color_r = r;
    }
    inline void setG(uint8_t g) noexcept {
        m_color_g = g;
    }
    inline void setB(uint8_t b) noexcept {
        m_color_b = b;
    }
    void setVal(unsigned int val) noexcept {
        m_color_a = (   (val & 0xFF000000) >> 24    );
        m_color_r = (   (val & 0x00FF0000) >> 16    );
        m_color_g = (   (val & 0x0000FF00) >> 8     );
        m_color_b =     (val & 0x000000FF);
    }

    ARGB32(unsigned int color32) noexcept {
        this->setVal(color32);
    }
    explicit ARGB32(ARGB16 argb16) noexcept {
        this->setVal(convert_ARGB16_to_ARGB32(argb16));
    }
    ARGB32(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept {
        this->setA(a);
        this->setR(r);
        this->setG(g);
        this->setB(b);
    }
    ARGB32& operator= (unsigned int color32) noexcept {
        this->setVal(color32);
        return *this;
    }
    ARGB32& operator= (ARGB16 argb16) noexcept {
         this->setVal(convert_ARGB16_to_ARGB32(argb16));
        return *this;
    }
    template<typename castT> operator castT() const noexcept {
        if (sizeof(castT) < sizeof(uint32_t))    // casting to a data type too small?
            return 0;
        return this->getVal();
    }

    void adjustBrightness(int percent) noexcept;
};

}

#endif // COLORS_H
