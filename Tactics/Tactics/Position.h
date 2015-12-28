#pragma once

namespace Game
{
    struct Position
    {
        Position();
        Position(int x, int y);
        int x;
        int y;
    };
	bool operator==(const Position& a, const Position& b);
	bool operator!=(const Position& a, const Position& b);

}   // ::Game

