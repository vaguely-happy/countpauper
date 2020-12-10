#include "stdafx.h"
#include "StaticEnvironment.h"
#include "Engine/Line.h"

namespace Physics
{
    
StaticEnvironment::StaticEnvironment(const Engine::Vector& size, const Engine::Vector& grid) :
    data(Size(int(std::round(size.x / grid.x)), int(std::round(size.y/ grid.y)), int(std::round(size.z / grid.z))), 0, grid)
{
    for (auto& v : data)
    {
        float temperature = 290.0;
        v.second = { &Material::air, temperature, float(Material::air.Density(PascalPerAtmosphere, temperature)) };
    }

}

size_t StaticEnvironment::Fill(const Engine::IVolume& v, const Material& m, double temperature)
{
    return data.Fill(v, m, temperature);
}

void StaticEnvironment::ApplyForce(const Engine::IVolume& c, const Engine::Vector& v) {}
void StaticEnvironment::ApplyForce(const Engine::Coordinate& c, double force) {}
void StaticEnvironment::Heat(const Engine::Coordinate& c, double energy) {}

void StaticEnvironment::ConnectChange(ChangeSignal::slot_type slot) {}

double StaticEnvironment::Density(const Engine::IVolume& c) const
{
    return data.Density(c);
}

double StaticEnvironment::Temperature(const Engine::IVolume& c) const
{
    return data.Temperature(c);
}

Engine::Vector StaticEnvironment::Force(const Engine::IVolume& c) const
{
    return data.Force(c);
}

const Material* StaticEnvironment::GetMaterial(const Engine::Coordinate& c) const
{
    // TODO: grid size in the environment or the data
    Position p(data.Grid(c));
    if (!data.IsInside(p))
        return nullptr;
    return &data.MaterialAt(p);
}

Engine::RGBA StaticEnvironment::Color(const Engine::Line& l) const
{
    Position p(data.Grid(l.b));
    if (!data.IsInside(p))
        return Engine::RGBA::transparent;
    return data[p].Color(l.Length());
}

void StaticEnvironment::Tick(double seconds)
{

}
}