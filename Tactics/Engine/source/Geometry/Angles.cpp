#include "Geometry/Angles.h"
#include <cmath>
#include <cassert>

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
        return (rad / PI)*180.0;
    }

    double Deg2Rad(double deg)
    {
        return (deg / 180.0) * PI;
    }

    double SphereVolume(double radius)
    {
        return pow(radius, 3) * PI * 4.0 / 3.0;
    }

    std::pair<double, double> FaceYawPitch(Vector direction, bool yawAxisZ)
    {
        if (yawAxisZ)
        {
            return std::make_pair(
                atan2(direction.x, direction.y),
                asin(direction.z)
            );
        }
        else
        {
            return std::make_pair(
                atan2(direction.x, direction.z),
                asin(-direction.y)
            );
        }
    }
}

