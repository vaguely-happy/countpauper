#include "stdafx.h"
#include "Direction.h"

namespace Game
{
    Direction::Direction() :
        value(Direction::Value::North)
    {

    }
    Direction::Direction(Direction::Value value) :
        value(value)
    {
    }
    
    Position Direction::Vector() const
    {
        return vector.at(value);
    }
    float Direction::Angle() const
    {
        return angle.at(value);
    }

    std::map<Direction::Value, Position> Direction::vector =
    {
        { Direction::Value::North, Position(0, -1) },
        { Direction::Value::East, Position(1, 0) },
        { Direction::Value::South, Position(0, 1) },
        { Direction::Value::West, Position(-1, 0) },
    };
    std::map<Direction::Value, float> Direction::angle =
    {
        { Direction::Value::North, M_PI * 0.5f },
        { Direction::Value::East, 0.0f },
        { Direction::Value::South, M_PI * -.5f },
        { Direction::Value::West, M_PI },
    };

    float ShortestTurn(float a, float b)
    {
        float result = a - b;
        if (result > M_PI)
        {
            return -2 * M_PI + result;
        }
        else if (result < -M_PI)
        {
            return 2 * M_PI + result;
        }
        return result;
    }

    float Rad2Deg(float rad)
    {
        return (rad / M_PI)*180.0f;
    }

}    // ::Game
