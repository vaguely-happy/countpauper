#include "stdafx.h"
#include <sstream>
#include "Engine/Maths.h"
#include "Game/Position.h"

namespace Game
{

    Position::Position() : x(0), y(0), z(0)
    {
    }
    Position::Position(int x, int y, int z) : x(x), y(y), z(z)
    {
    }

    unsigned Position::ManDistance(const Position& other) const
    {
        return std::abs(other.x - x) +
            std::abs(other.y - y) +
            std::abs(other.z - z);
    }

    float Position::Size() const
    {
        return std::sqrtf(Engine::Sqr(float(x)) + Engine::Sqr(float(y))+Engine::Sqr(float(z)));
    }

    unsigned Position::SizeEl() const
    {
        return static_cast<unsigned>(std::round(std::sqrtf(
            Engine::Sqr(float(HorizontalEl*x)) +
            Engine::Sqr(float(HorizontalEl*y)) +
            Engine::Sqr(float(VerticalEl*z)))
        ));
    }

    float Position::Distance(const Position& other) const
    {
        return std::sqrtf(Engine::Sqr(float(other.x - x)) + Engine::Sqr(float(other.y - y)) + Engine::Sqr(float(other.z -z)));
    }

    unsigned Position::DistanceEl(const Position& other) const
    {
        return static_cast<unsigned>(std::round(std::sqrtf(
            Engine::Sqr(float(HorizontalEl*(other.x - x))) +
            Engine::Sqr(HorizontalEl * (other.y - y)) +
            Engine::Sqr(VerticalEl* (other.z - z))
            )+0.0132)); // special bias so DistanceEl(3,1) > DistanceEl(3,0) 
    }

    Position& Position::operator+=(const Position& delta)
    {
        x += delta.x;
        y += delta.y;
        z += delta.z;
        return *this;
    }
    Position& Position::operator-=(const Position& delta)
    {
        x -= delta.x;
        y -= delta.y;
        z -= delta.z;
        return *this;
    }
    Position::operator bool() const
    {
        return x != 0 || y != 0 || z != 0;
    }

    Position operator+(const Position& a, const Position& b)
    {
        Position o(a);
        o += b;
        return o;
    }
    Position operator-(const Position& a, const Position& b)
    {
        Position o(a);
        o -= b;
        return o;
    }

    std::wstring Position::to_wstring() const
    {
        std::wstringstream ss;
        ss << L"(" << x << L", " << y << L", " << z << L")";
        return ss.str();
    }
    bool operator==(const Position& a, const Position& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    bool operator!=(const Position& a, const Position& b)
    {
        return !operator==(a, b);
    }

} // ::Game