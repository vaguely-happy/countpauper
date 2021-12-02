#pragma once
#include <string>
#include "Material.h"
#include <signal.h>
#include <functional>

namespace Engine { 
    struct Line; 
    struct Coordinate;
    struct Vector;  
    class IVolume; 
    struct AABB;
}

namespace Physics
{
using Function = std::function<double(double)>;


class IEnvironment
{
public:
    // Creation
    virtual size_t Fill(const Engine::IVolume& v, const Material& m, double temperature, std::optional<double> density=std::optional<double>()) = 0;
    virtual void Constrain(const Engine::IVolume& v, const Material& m, double temperatire, Function density) = 0;
    virtual void ApplyForce(const Engine::IVolume& c, const Engine::Vector& v)=0;
    virtual void ApplyForce(const Engine::Coordinate& c, double force) = 0;
    virtual void Heat(const Engine::Coordinate& c, double energy) = 0;
    using ChangeSignal = is::signals::signal<void(const Engine::AABB&)>;

    virtual is::signals::connection ConnectChange(ChangeSignal::slot_type slot) = 0;
    virtual double Density(const Engine::IVolume& c) const =0;
    virtual double Temperature(const Engine::IVolume& c) const = 0; // kelvin
    virtual Engine::Vector Force(const Engine::IVolume& c) const = 0;
    virtual const Material* GetMaterial(const Engine::Coordinate& c) const = 0;
    virtual Engine::RGBA Color(const Engine::Line& l) const = 0;
    virtual double Measure(const Material* material) const = 0;
    virtual std::wstring Statistics() const = 0;

    virtual void Tick(double seconds) = 0;
};
}
