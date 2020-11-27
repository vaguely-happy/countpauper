#include "stdafx.h"
#include "StaticEnvironment.h"

namespace Game
{
    
StaticEnvironment::StaticEnvironment(const Size& size) :
    data(size)
{
    for (auto& v : data)
    {
        float temperature = 290.0;
        v.second = { &Material::air, temperature, float(Material::air.Density(PascalPerAtmosphere, temperature)) };
    }

}

void StaticEnvironment::Fill(const Engine::IVolume& v, const Material& m)
{
    data.Fill(v, m, 0);
}

void StaticEnvironment::ApplyForce(const Engine::IVolume& c, const Engine::Vector& v) {}
void StaticEnvironment::ApplyForce(const Engine::Coordinate& c, double force) {}
void StaticEnvironment::Heat(const Engine::Coordinate& c, double energy) {}

void StaticEnvironment::ConnectChange(ChangeSignal::slot_type slot) {}

double StaticEnvironment::Density(const Engine::IVolume& c) const
{
    return 0;
}

double StaticEnvironment::Temperature(const Engine::IVolume& c) const
{
    return 0;
}

Engine::Vector StaticEnvironment::Force(const Engine::IVolume& c) const
{
    return Engine::Vector();
}

const Material* StaticEnvironment::GetMaterial(const Engine::Coordinate& c) const
{
    // TODO: grid size in the environment or the data
    Position p(int(c.x), int(c.y), int(c.z));
    if (!data.IsInside(p))
        return nullptr;
    return &data.MaterialAt(p);
}
void StaticEnvironment::Tick(double seconds)
{

}
}