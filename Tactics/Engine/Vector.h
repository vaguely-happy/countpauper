#pragma once

namespace Engine
{
struct Coordinate;

struct Vector
{
    Vector() = default;
    Vector(double x, double y, double z) :
        x(x),
        y(y),
        z(z)
    {
    }
    Vector(const Vector& v) = default;

    Vector(const Coordinate& c);

    Vector& operator=(const Vector& o);
    operator bool() const;
    double Length() const;
    double LengthSquared() const;
    Vector Normal() const;

    bool operator==(const Vector& o) const;
    Vector& operator*=(double factor);
    Vector& operator/=(double factor);
    Vector& operator+=(const Vector& v);
    Vector& operator-=(const Vector& v);
    Vector operator-() const;
    Vector& operator*=(const Vector& cross);

    double Dot(const Vector& v) const;

    double x;
    double y;
    double z;
};
Vector operator*(const Vector& v, double factor);
Vector operator/(const Vector& v, double factor);
Vector operator*(const Vector& a, const Vector& b);
Vector operator*(const Vector& v, double factor);
Vector operator+(const Vector& a, const Vector& b);
Vector operator-(const Vector& a, const Vector& b);
Vector operator-(const Coordinate& a, const Coordinate& b);

} // ::Engine
