#pragma once
#include <cstdint>

namespace Engine
{
    struct RGBA
    {
        using Component = uint8_t;

        RGBA(Component r, Component g, Component b, Component a=255) :
            r(r), g(g), b(b), a(a)
        {
        }
        RGBA(uint32_t c) :
            r(c & 0xFF),
            g((c >> 8) & 0xFF),
            b((c >> 16) & 0xFF),
            a((c >> 24))
        {
        }
        Component r, g, b, a;
        void Render() const;
        RGBA operator*=(double factor);
        RGBA Translucent(double factor) const;
    };

    RGBA operator*(const RGBA& color, double factor);

}   // ::Engine