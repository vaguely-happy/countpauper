#include "Rendering/Text.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <cassert>
#include <algorithm>

namespace Engine
{

    unsigned Font::nextBase = 1000;
    Font Font::basic;

    Font::Font() :
        base(nextBase),
        height(16)
    {
        #ifdef WIN32
        auto hdc = GetDC(nullptr);
        HFONT font = CreateFont(height, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, 0,
            0, 0, 0, "Arial"); // can only use true type fonts
        //  this one is 40 points in size
        SelectObject(hdc, font);

        wglUseFontBitmaps(hdc, 0, 127, base);
        nextBase += 128;    // TODO can be optimized to 100 main characters (32 to 128-32)
        ReleaseDC(nullptr, hdc);
        #else
        assert(false);
        #endif
    }

    void Font::Select() const
    {
        glListBase(base);
    }

    unsigned Font::Height() const
    {
        return height;
    }

    void glText(std::string_view text)
    {
        glRasterPos3i(0, 0, 0); // set start position
        for(auto c : text)
        {
            glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
    }

}
