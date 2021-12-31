﻿#include "stdafx.h"
#include "Material.h"
#include "Engine/from_string.h"

namespace Physics
{

//                               Name,         Color,                      Melt,   Boil, Density, molar mass,   Viscosity,  Conduct,    thermalCond, Capacity    surfaceTensiom, youngModulus,  Opacity,         
const Material Material::vacuum{ L"Vacuum",    Engine::RGBA(0x00000000),   0,      0,      0,      0,           0,          0,          0,          0,          0,              0,             0.0 };
const Material Material::air   { L"Air",       Engine::RGBA(0xFFA08040),   60,     80,     1.225,  29,          18e-6,      29.2,       0.024,      1.012,      10e-3,          1e9,           0.01 };     // 28.964g/mol because n2&o2 diatomic
const Material Material::soil  { L"Soil",      Engine::RGBA(0xFF20FF20),   0,      0,      1600,   65,          10e3,       0.4,        1,          2.0,        0.7,            1e7,           10 };     // 65g/mol, based on 0% humidity. Part SiO2, N2 and proteins
const Material Material::stone { L"Stone",     Engine::RGBA(0xFFA0A0A0),   1986,   3220,   2648,   60,          10e21,      10,         1.8,        0.790,      0.8,            1e10,          10 };     // for now 100% silicon dioxide, 60 g/mol
const Material Material::water { L"Water",     Engine::RGBA(0xFFFF6010),   273,    373,    1000,   18,          8.9e-4,     0.6065,     0.6,        4.1813,     0.07,           9e9,           6.9 / 1000 };     // H2O 18 g/mol
                                                                                                                                            
// redundant for now VoxelMap::Material VoxelMap::Material::sand  { L"Sand",      1986,   3220,   2648,        10,            0.0           };     // silicon dioxide, 60 g/mol

bool Material::Solid(double temperature) const
{
    return temperature < melt;
}

bool Material::Fluid(double temperature) const
{
    return temperature >= melt && temperature < boil;
}

bool Material::Gas(double temperature) const
{
    return temperature >= boil;
}

double Material::Evaporation(double temperature, double pressure, double humidity, double windSpeed) const
{
    // https://en.wikipedia.org/wiki/Clausius%E2%80%93Clapeyron_relation
    // that's too hard so approximation, which currently uses constants that are water specific
    assert(this == &Material::water);
    // https://www.engineeringtoolbox.com/evaporation-water-surface-d_690.html
    // Θ A (xs - x) / 3600  
    // Θ = 44 (35+1m/s * 19) 
    // A = 1 m2 (because that's the unit 

    // assuming v= 1m/s wind to carry vapor away, could take velocity from air block when it's tracked
    const double theta = 25 + windSpeed * 19.0;
    const double A = 1;
    const double secondsPerHour = 3600.0;
    // Humidity ratio(xs) at this temperature: https://www.engineeringtoolbox.com/humidity-ratio-air-d_686.html
    double pws = exp(77.3450 + 0.0057 * temperature - 7235 / temperature) / pow(temperature, 8.2);
    // Using 1 bar atmoshpheric pressure here. It should be the pressure of the air evaporating t;
    double xs = 0.62198 * pws / (pressure - pws);
    // current humidity assumed to be constant 1g/kg, since it's not tracked in air currently. 
    // perhaps it could be based on overlapping vapor particles or 
    // even packed voxels should track humidity in air (and mineral) and dirt is just wet + mineral (assume bio mass grows instantly) 
    const double x = humidity;
    double evaporation = theta * A * (xs - x) / 3600;
    return 1000.0 * evaporation;
}


double Material::Density(double pressure, double temperature) const
{
    if (temperature > boil)
    {
        return (molarMass * pressure) / (temperature * IdealGasConstant);
    }
    else
    {
        return normalDensity;  // incompressible, also as liquid
    }
}

std::ostream& operator<<(std::ostream& stream, const Physics::Material& material)
{
    stream << Engine::from_string<std::string>(material.name);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Physics::Material* material)
{
    if (!material)
        stream << "None";
    else
        stream << *material;
    return stream;
}

}
