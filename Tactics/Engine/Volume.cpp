#include "stdafx.h"
#include "Volume.h"
#include "Coordinate.h"
#include "Matrix.h"
#include <numeric>

namespace Engine
{
AABB Point::GetBoundingBox() const
{
    return AABB(c, c);
}

double Point::Distance(const Coordinate& p) const
{
    return (c - p).Length();
}

AABB Sphere::GetBoundingBox() const
{
    Vector v(radius, radius, radius);
    return AABB(center - v, v * 2.0);
}

double Sphere::Distance(const Coordinate& p) const
{
    double d = (center - p).Length() - radius;
    return d;
}

double AABox::Distance(const Coordinate& p) const
{
    if (GetBoundingBox().Contains(p))
    {
        Vector d(std::min(p.x - x.begin, x.end - p.x),
            std::min(p.y - y.begin, y.end - p.y),
            std::min(p.z - z.begin, z.end - p.z));
        return -std::min(std::min(d.x, d.y), d.z);
    }
    else
    {
        Vector d(std::max(0.0, std::max(x.begin - p.x, p.x - x.end)),
            std::max(0.0, std::max(y.begin - p.y, p.y - y.end)),
            std::max(0.0, std::max(z.begin - p.z, p.z - z.end)));
        return d.Length();
    }
}


Cylinder::Cylinder(const Line& axis, double dy, double dz) :
    scale(axis.Length(),dy,dz),  // unit cylinder unit length and unit radius in both directions
    origin(axis.a)
{
    Vector v(axis);
    // axis inverted because we need the conjugate, the rotation that led to this orientation
    Quaternion zrot(Vector(0, 0, -1), atan2(v.y, v.x));
    v *= -zrot;
    Quaternion yrot(Vector(0, -1, 0), atan2(v.z, v.x));
    orientation = zrot * yrot;

}

AABB Cylinder::GetBoundingBox() const
{
    AABB box(Coordinate(0, -1, -1), Vector(1, 2, 2));   // TODO transform
    box *= Matrix::Scale(scale);
    box *= orientation.Matrix();
    box *= Matrix::Translation(origin);

    return box;
}

double Cylinder::Distance(const Coordinate& p) const
{
    // inverse transform to x-axis aligned cylinder : a=origin (0,0,0)
    Coordinate tp = p - origin;
    tp *= orientation.Conjugate().Matrix();
    // scale is not inversed, because it can't handle 0 sizes and the result will need to be transformed back in a weird way. Instead the unit cylinder is scaled
    double dy = scale.y, dz = scale.z;
    auto coefficient = tp.x / scale.x;
    auto projection = Vector(tp.x,0,0);  // p
    Vector projectionVector(0,-tp.y, -tp.z);  // pv
    // NB: this is not exactly correct, the nearest point on an elipse is not on the line between point and the center
    //  but there is no analytical solution so this is faster
    double cylinderRadius;
    if ((tp.y == 0) && (tp.z == 0))
    {
        cylinderRadius = std::min(scale.y, scale.z);
    }
    else
    {
        double a = atan2(tp.z, tp.y);
        cylinderRadius = Lerp(dz, dy, std::abs(cos(a)));
    }
    auto axisDistance = projectionVector.Length(); // |pv|

    // B          E         D
    //    |-------+----|
    // A  |   E'  | pv | } cylinderRadius
    //c<0 a-------p----b   c>1 
    //    |            |   C
    //    |------------|

    if (coefficient <= 0)
    {
        auto planeDistance = -tp.x;
        if (axisDistance < cylinderRadius)
        {   // A: in tube, distance to front start plane
            return planeDistance;
        }
        else
        {   // B: outside of tube, distance to start circle
            auto tubeDistance = axisDistance - cylinderRadius;
            return sqrt(planeDistance*planeDistance + tubeDistance * tubeDistance);
        }
    }
    else if (coefficient >= 1)
    {
        auto planeDistance = tp.x - scale.x;
        if (axisDistance < cylinderRadius)
        {   // C: in tube, distance to front start plane
            return planeDistance;
        }
        else
        {   // D: outside of tube, distance to end circle
            auto tubeDistance = axisDistance - cylinderRadius;
            return sqrt(planeDistance*planeDistance + tubeDistance * tubeDistance);
        }
    }
    else
    {   // projection inside the cylinder. point either outside (E) or inside the cylinder (E') 
        auto planeDistance = std::min(tp.x, scale.x - tp.x);
        return std::min(axisDistance - cylinderRadius, planeDistance);
    }
    return 0;
}

AABB Intersection::GetBoundingBox() const
{
    AABB box = AABB::infinity;
    for (const auto& av : volumes)
    {
        if (const IVolume* v = std::any_cast<IVolume>(&av))
        {
            box &= v->GetBoundingBox();
        }
    }
    return box;
}

double Intersection::Distance(const Coordinate& p) const 
{
    return std::accumulate(volumes.begin(), volumes.end(), std::numeric_limits<double>::infinity(), [&p](double value, const std::any& av)
    {
        if (const IVolume* v = std::any_cast<IVolume>(&av))
        {
            return std::max(value, v->Distance(p));
        }
        else
        {
            return value;
        }
        
    });
}


}