#include "stdafx.h"
#include "Geometry.h"

namespace Engine
{
    double ShortestTurn(double from, double to)
    {
        double result = to - from;
        if (result > PI)
        {
            return -2 * PI + result;
        }
        else if (result < -PI)
        {
            return 2 * PI + result;
        }
        return result;
    }

    double Rad2Deg(double rad)
    {
        return (rad / PI)*180.0f;
    }

}

