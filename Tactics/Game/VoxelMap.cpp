#include "stdafx.h"
#include <numeric>
#include "VoxelMap.h"
#include <gl/GL.h>
#include "Engine/Image.h"
#include "Engine/Error.h"
#include "Engine/Coordinate.h"
#include "Engine/Geometry.h"
#include "Engine/Line.h"
#include "Engine/Timer.h"
#include "Engine/Maths.h"
#include "Engine/Random.h"
#include "Engine/Drawing.h"
#include "Engine/Text.h"
#include "Engine/AxisAlignedBoundingBox.h"
#include <string>

namespace Game
{


double VoxelMap::Material::Density(double pressure, double temperature) const
{
    if (temperature > boil)
    {
        return (molarMass * pressure ) / (temperature * IdealGasConstant);
    }
    else
    {
        return normalDensity;  // incompressible, also as liquid
    }
}

VoxelMap::VoxelMap() :
    voxels(),
    time(0),
    gravity(-10.0),
    planetRadius(6.371e6),  // assume earth sized planet
    atmosphereRadius(2e5),  // assume earth atmosphere
    atmosphericTemperature(273.15f)
{
    World(planetRadius);
    wind = Engine::Vector(0.3, 0.2, 0.1);
}



void VoxelMap::Space(unsigned x, unsigned y, unsigned z)
{
    voxels = Data(x, y, z);
}

void VoxelMap::World(double radius)
{
    planetRadius = radius;
    for (unsigned x = 0; x < voxels.Longitude(); ++x)
    {
        for (unsigned y = 0; y < voxels.Latitude(); ++y)
        {
            voxels.Set(Position(x, y, -1),
                Material::stone,
                (Material::stone.melt + Material::stone.boil) / 2.0,
                PascalPerAtmosphere);
        }
    }
    constexpr double coreDensity = 5000; // g/L approximate density of earth. liquid iron is 7000 
    constexpr double G = 6.67430e-11 * 0.001;  // m^3 / g * s^2
    double worldMass = float(Engine::SphereVolume(planetRadius) * 1000.0 * coreDensity);
    // surface gravity: https://en.wikipedia.org/wiki/Surface_gravity
    gravity = G * worldMass / (planetRadius*planetRadius);
}



float VoxelMap::AtmosphericTemperature(int z) const
{
    double atmosphericLapse = atmosphericTemperature / atmosphereRadius;
    return float(atmosphericTemperature + atmosphericLapse * z * VerticalEl * MeterPerEl);
}

float VoxelMap::AtmosphericPressure(int z) const
{
    return float(PascalPerAtmosphere * (1.0-(z*VerticalEl*MeterPerEl)/ atmosphereRadius));
}

void VoxelMap::Air(double temperature, double meters)
{
//    atmosphere.mass = float((Engine::SphereVolume(meters + planetRadius) - Engine::SphereVolume(planetRadius))*
//        1000.0 * atmosphere.material->normalDensity * 0.5);    // TODO: density isn't even over the whole atmosphere. assumed average
    atmosphericTemperature = float(temperature);
    atmosphereRadius = meters;
    double atmorphericLapse = float(-temperature / atmosphereRadius);
    Position p;
    for (p.x = -1; p.x <= int(voxels.Longitude()); ++p.x)
    {
        for (p.y = -1; p.y <= int(voxels.Latitude()); ++p.y)
        {
            for (p.z = -1; p.z <= int(voxels.Altitude()); ++p.z)
            {
                voxels.Set(p, Material::air,
                    AtmosphericTemperature(p.z),
                    AtmosphericPressure(p.z));
            }
        }
    }
}

void VoxelMap::Water(int level, double temperature)
{
    Position p;
    for (p.x = -1; p.x <= int(voxels.Longitude()); ++p.x)
    {
        for (p.y= -1; p.y <= int(voxels.Latitude()); ++p.y)
        {
            for (p.z = -1; p.z < level; ++p.z)
            {
                double pressure = AtmosphericPressure(p.z) + (level - p.z)*VerticalEl*MeterPerEl * 10.33; // TODO: calculate based on water material
                voxels.Set(p,
                    Material::water,
                    temperature,
                    pressure);
            }
        }
    }
}

void VoxelMap::Hill(const Engine::Coordinate& p1, const Engine::Coordinate& p2, float stddev)
{
    Engine::Line ridge_line(Engine::Coordinate(p1.x, p1.y, 0),
                    Engine::Coordinate(p2.x, p2.y, 0));
    // offset the hill for external coordinates excluding boundaries
    Position p;
    for (p.x = -1; p.x <= int(voxels.Longitude()); ++p.x)
    {
        for (p.y = -1; p.y <= int(voxels.Latitude()); ++p.y)
        {
            Engine::Coordinate c(float(p.x)+0.5f, float(p.y)+0.5f, 0);
            double distance = Engine::Distance(c, ridge_line);
            double interpolation_factor = std::max(0.0,std::min(1.0, ridge_line.Vector().Dot(c - ridge_line.a) / ridge_line.Vector().LengthSquared()));
            double height = Engine::Lerp(double(p1.z), double(p2.z), interpolation_factor) * Engine::Gaussian(distance, stddev);
            int maxZ = 1+int(std::round( height ));
            maxZ = std::min(maxZ, int(voxels.Altitude()));
            for (p.z = 0; p.z < maxZ; ++p.z)
            {
                voxels.Set(p,
                    Material::stone,
                    atmosphericTemperature, // TODO: decrease from air increase to lava
                    PascalPerAtmosphere);   // TODO: increase due to stone depth, NB: can be already overlapping stone layer. just dont place those
            }
        }
    }
}

unsigned VoxelMap::Latitude() const
{
    return voxels.Latitude();
}
unsigned VoxelMap::Longitude() const
{
    return voxels.Longitude();
}

Directions VoxelMap::Visibility(const Position& p) const
{
    Directions result;
    for (const Direction& direction : Direction::all)
    {
        auto neighbourPosition = p + direction.Vector();

        if (!voxels.IsInside(neighbourPosition) || 
            !voxels[neighbourPosition].Opaque())
        {
            result|=direction;
        }
    }
    return result;
}

Square VoxelMap::At(const Position& p) const
{
    if (p.x < 0)
        return Square();
    if (p.y < 0)
        return Square();
    if (unsigned(p.x) >= Longitude())
        return Square();
    if (unsigned(p.y) >= Latitude())
        return Square();
    for (int i = std::min(p.z, int(voxels.Altitude())); i > 0; --i)
    {
        const auto& v = voxels[Position(p.x, p.y, i)];
        if (v.Solid())
        {
            Square s = v.Square(i);
            return s;
        }
    }
    return voxels[Position(p.x,p.y,-1)].Square(0); // welcome to hell
}
/*
void VoxelMap::ComputeForces(double seconds)
{
    // Flow
    for (auto& v : voxels)
    {
        if (v.boundary)
        {
        }
        else
        {   // en.wikipedia.org/wiki/Viscosity#Dynamic_and_kinematic_viscosity
            double viscosity = v->Viscosity() / v->Density();
            v->flow -= v->flow * viscosity * seconds;
            auto pressure = v->Pressure();
            for (auto d : Direction::all)
            {
                auto neightbourPosition = v.position + d.Vector();
                // presssure (Pa) is N/m2
                auto& neighbour = voxels[neightbourPosition];
                if (!neighbour.Solid())
                {   
                    // Pressure gradient (Pa) is N/m^2. Times surface area is newton
                    double force = d.Surface() * (pressure - neighbour.Pressure());  // divide over distance?
                    // Law of motion. Acceleration = Force/mass (kg)
                    double acceleration = (force / (v->Mass()*1e-3));
                    //  multiplied with time it's the change in velocity
                    acceleration *= seconds;
                    Engine::Vector dir(d.Vector().x, d.Vector().y, d.Vector().z);
                    // Not newtonian, spread force over two grids
                    v->flow += dir * acceleration * 0.5;
                    double neighbourAcceleration = (force * seconds / neighbour.Mass()*1e-3);
                    neighbour.flow += dir * neighbourAcceleration *0.5;
                    if (d == Direction::down)
                    {
                        v->flow += Engine::Vector(0, 0, -gravity)*seconds;
                    }
                }
                else
                {   // normal force
                    Engine::Vector normal(-d.Vector().x, -d.Vector().y, -d.Vector().z);
                    auto dot = normal.Dot(v->flow);
                    if (dot < 0)
                    {   // TODO: check the force against the strength of the surface
                        v->flow -= normal * dot;
                    }
                }
            }
        }
    }
}
*/

/*
void VoxelMap::Diffuse(double seconds)
{

    const Data original = voxels;   // TODO can make more efficient partial copy
    auto it = original.begin();
    for (auto& v : voxels)
    {
        if (v.boundary)
        {   // Complete immediate diffusion 
            //  TODO: could do it over time and surface of boundary but then this wouldn't be the boundary
            v->temperature = AtmosphericTemperature(v.position.z);
            double pressure = AtmosphericPressure(v.position.z);
            v->density = float(v->material->Density(pressure, v->temperature));
        }
        else
        {
            for (auto d : Direction::all)
            {   
                const auto& old = **it;
                auto neighbourPosition = v.position + d.Vector();
                const auto& oldNeighbour = original[neighbourPosition];
                if (old.material == oldNeighbour.material)
                {
                    double coefficient = old.DiffusionCoefficient(oldNeighbour);
                    if (coefficient != 0)
                    {   // Simplified Diffusion equation: https://en.wikipedia.org/wiki/Diffusion_equation
                        // dDensity/dt = DiffusionFactor * density
                        //  since volume is (currently) equal between each voxel, dMass = DiffusionFactor * mass * dt
                        auto diffusionFactor = coefficient * seconds * d.Surface();
                        assert(diffusionFactor > 0);   // don't know what negative diffusion means 
                        auto deltaDensity = float((old.density - oldNeighbour.density) * diffusionFactor);
                        auto deltaTemperature = float((old.temperature - oldNeighbour.temperature) * diffusionFactor);

                        v->density -= deltaDensity;
                        // Thermal convenction: https://en.wikipedia.org/wiki/Thermal_conduction
                        v->temperature -= deltaTemperature;
                        if (!voxels.IsBoundary(neighbourPosition))
                        {
                            auto& neighbour = voxels[neighbourPosition];
                            neighbour.density += deltaDensity;
                            neighbour.temperature += deltaTemperature;
                        }
                    }
                }
                else
                {
                    // dont diffuse different materials yet
                    // Always diffuse heat
                    // https://en.wikipedia.org/wiki/Thermal_conduction#Fourier's_law
                    auto flux = v->material->conductivity * (old.temperature - oldNeighbour.temperature);
                    auto deltaJoule = flux * seconds * d.Surface();
                    auto& neighbour = voxels[neighbourPosition];
                    v->temperature -= float(deltaJoule / (old.material->heatCapacity*old.Mass()));
                    neighbour.temperature += float(deltaJoule / (oldNeighbour.material->heatCapacity*oldNeighbour.Mass()));
                }
            }
        }
        ++it;
    }
}
*/

/*

Position VoxelMap::MaxFlow() const
{
    double maxSqrFlow = 0;
    Position maxLocation;
    for (auto& v : voxels)
    {
        double sqr = v->flow.LengthSquared();
        if (sqr > maxSqrFlow)
        {
            maxSqrFlow = sqr;
            maxLocation = v.position;
        }
    }
    return maxLocation;
}
*/

unsigned VoxelMap::WindForce() const
{
    return 0;
    /*
    const auto& voxel = voxels[MaxFlow()];
    auto speed = voxel.flow.Length();   // m/s
    constexpr std::array<double, 12> beaufortScale = { 0.2, 1.5, 3.3, 5.4, 7.9, 10.7, 13.8, 17.1, 20.7, 24.4, 28.4, 32.6 };
    unsigned beaufort = 0;
    for (auto force : beaufortScale)
    {
        if (speed <= force)
            break;
        beaufort++;
    }
    return beaufort;
    */
}

double VoxelMap::Volume() const
{
    return voxels.Longitude() * voxels.Latitude() * voxels.Altitude() * LiterPerBlock;
}

double VoxelMap::Mass(const VoxelMap::Material& material) const
{
    return std::accumulate(voxels.begin(), voxels.end(), 0.0, [&material](double runningTotal, const decltype(voxels)::value_type& v)
    {
        if (&v.material == &material)
            return runningTotal + v.Mass();
        else
            return runningTotal;
    });
}

double VoxelMap::Temperature(const Material& material) const
{
    auto heat = std::accumulate(voxels.begin(), voxels.end(), 0.0, [&material](double runningTotal, const decltype(voxels)::value_type& v)
    {
        if (&v.material == &material)
            return runningTotal + v.Mass() * v.temperature;
        else
            return runningTotal;
    });

    return heat / Mass(material);
}

double VoxelMap::Mass() const
{
    return Mass(Directions(0xFFFF));
}

double VoxelMap::Mass(Directions boundaries) const
{
    return std::accumulate(voxels.begin(), voxels.end(), 0.0, [&boundaries](double runningTotal, const decltype(voxels)::value_type& v)
    {
        return runningTotal + v.Mass();
    });
}
/*
void VoxelMap::Advection(double seconds)
{

    const Data original = voxels;   
    auto it = original.begin();
    Engine::Vector maxFlow = voxels[MaxFlow()].flow * seconds;
    // need to divide in steps if advection is larger than grid size
    assert(maxFlow.x < HorizontalEl*MeterPerEl);    // should be good to 100m/s
    assert(maxFlow.y < HorizontalEl*MeterPerEl);
    assert(maxFlow.z < VerticalEl*MeterPerEl);      // this may be a problem with explosions

    for (auto& org : original)
    {
        if (org.boundary)
        {   
        }
        else
        {
            // Axis aligned bounding box of the destination volume
            //  relative to the original's position
            auto displacement = org->flow*seconds;
            Engine::AABB newBB(Engine::Coordinate(displacement.x, displacement.y, displacement.z),
                Engine::Vector(1, 1, 1));

            auto& source = voxels[org.position];
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    for (int z = -1; z <= 1; ++z)
                    {
                        Position targetPosition(org.position.x + x, org.position.y + y, org.position.z + z);
                        auto& target = voxels[targetPosition];
                                
                        if (target.material == org->material)
                        {
                            // Axis aligned bounding box of the target volume, also relative
                            Engine::AABB volumeBB(Engine::Coordinate(x, y, z), Engine::Vector(1, 1, 1));
                            Engine::AABB overlap = newBB & volumeBB;
                            auto overlapVolume = overlap.Volume();
                            if (overlapVolume > 0)
                            {
                                target.density += float(overlapVolume * org->density);
                                target.temperature += float(overlapVolume * org->temperature);
                                target.flow += org->flow * overlapVolume;

                                source.density -= float(org->density * overlapVolume);
                                source.temperature -= float(org->temperature * overlapVolume);
                                source.flow -= org->flow * overlapVolume;
                            }
                        }
                    }
                }
            }
        }
    }
}
*/


void VoxelMap::FluxBoundary()
{
    for (auto d : Direction::all)
    {
        for (auto u : voxels.U().BoundaryCondition(d))
        {
            voxels.U()[u.first] = wind.x;
        }
        for (auto v : voxels.V().BoundaryCondition(d))
        {
            voxels.V()[v.first] = wind.y;
        }
        for (auto w : voxels.W().BoundaryCondition(d))
        {
            voxels.W()[w.first] = wind.z;
        }
    }
}

void VoxelMap::GridBoundary()
{

}

void VoxelMap::Flow(double dt)
{
    const Data::Flux oU = voxels.U();
    const Data::Flux oV = voxels.V();
    const Data::Flux oW = voxels.W();
    // U Flow
    Position p;
    for (p.x = 1; p.x < int(voxels.Longitude()); ++p.x)
    {
        for (p.y = 0; p.y < int(voxels.Latitude()); ++p.y)
        {
            for (p.z = 0; p.z < int(voxels.Altitude()); ++p.z)
            {
                constexpr double dx = HorizontalEl*MeterPerEl;
                constexpr double dy = HorizontalEl*MeterPerEl;
                constexpr double dz = VerticalEl*MeterPerEl;

                double duudx = (Engine::sqr(oU(p.x + 1, p.y, p.z)) - Engine::sqr(oU(p.x - 1, p.y, p.z))) / (2.0*dx);
                double duvdy = 0.25*((oU(p.x, p.y, p.z) + oU(p.x, p.y + 1, p.z)) * (oV(p.x, p.y, p.z) + oV(p.x + 1, p.y, p.z))
                    - (oU(p.x, p.y, p.z) + oU(p.x, p.y - 1, p.z)) * (oV(p.x+1, p.y-1, p.z) + oV(p.x, p.y-1, p.z)))/dy;
                double duwdz = 0.25*((oU(p.x, p.y, p.z) + oU(p.x, p.y, p.z + 1)) * (oW(p.x, p.y, p.z) + oW(p.x + 1, p.y, p.z))
                    - (oU(p.x, p.y, p.z) + oU(p.x, p.y, p.z - 1)) * (oW(p.x + 1, p.y, p.z - 1) + oW(p.x, p.y, p.z - 1))) / dz;
                double dpdx = (voxels.Density(p) - voxels.Density(Position(p.x - 1, p.y, p.z)))/dx;
                double Reynolds = 100.0; 
                double diff = (1.0/Reynolds) *
                    ((oU(p.x + 1, p.y, p.z) - 2.0*oU(p.x, p.y, p.z) + oU(p.x - 1, p.y, p.z)) / (dx*dx) +
                     (oU(p.x, p.y + 1, p.z) + 2.0*oU(p.x, p.y, p.z) + oU(p.x, p.y - 1, p.z)) / (dy*dy) +
                     (oU(p.x, p.y, p.z + 1) + 2.0*oU(p.x, p.y, p.z) + oU(p.x, p.y, p.z - 1)) / (dz*dz));
                voxels.U()[p] += float(dt * (diff - duudx - duvdy - duwdz - dpdx));
            }
        }
    }
    FluxBoundary();
}

void VoxelMap::Continuity(double seconds)
{   
    for (auto it = voxels.begin(); it!=voxels.end(); ++it)
    {
        auto fluxGradient = voxels.FluxGradient(it.position);
        auto p = voxels.Density(it.position);
        auto dP = fluxGradient.x * fluxGradient.y * fluxGradient.z * seconds;
        voxels.SetDensity(it.position, float(p + dP));
    }
    GridBoundary();
}


void VoxelMap::Tick(double seconds)
{
    // Fluid dynamics:
    //  https://en.wikipedia.org/wiki/Fluid_dynamics
    //  With incompressible flow, as long as it stays under 111m/s (mach 0.3) is simpler
 
    Engine::Timer start;
    Flow(seconds);
    Continuity(seconds);
   
    time += seconds;
    double performance = start.Seconds();

    OutputDebugStringW((std::wstring(L"Tick in ") + std::to_wstring(performance*1000.0)+L"ms " 
        L"Wind=" + std::to_wstring(WindForce()) + L" bft. "+
        L"Mass=" + std::to_wstring(Mass()) + L"g " +
        L"Air Temp=" + std::to_wstring(int(Temperature(Material::air)-273.15)) + L"�C "+
        L"\n").c_str());
}

void VoxelMap::Render() const
{
    //RenderPretty();
    RenderAnalysis();
}


void VoxelMap::RenderPretty() const
{
    assert(glIsEnabled(GL_DEPTH_TEST));

    // Draw opaque
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    for (auto& v : voxels)
    {
        glPushMatrix();
        glTranslated(v.position.x, v.position.z*MeterPerEl, v.position.y); // offset by -1,-1,-1 for boundary
        if (v.Opaque())
        {
            if (auto visibility = Visibility(v.position))
            {
                v.Render(v.position, visibility);
            }
        }
        glPopMatrix();
    }

    // translucent pass
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    for (auto& v : voxels)
    {
        glPushMatrix();
        glTranslated(v.position.x, v.position.z*MeterPerEl, v.position.y); // offset by -1,-1,-1 for boundary
        if (!v.Opaque() && !v.Transparent())
        {
            auto visibility = Visibility(v.position);
            if (!visibility.empty())
            {
                v.Render(v.position, visibility, false);
            }
        }
        glPopMatrix();
    }
    Engine::CheckGLError();
}


void VoxelMap::RenderAnalysis() const
{
    assert(glIsEnabled(GL_DEPTH_TEST));

    // Draw opaque
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    for (auto& v : voxels)
    {
        glPushMatrix();
        glTranslated(v.position.x, v.position.z*MeterPerEl, v.position.y);
        v.Render(v.position, Directions(0xFFFF), Engine::RGBA::red);
        glPopMatrix();
    }


    glColor3d(1, 1, 1);
    Position p;
    for (auto u : voxels.U())
    {
        glPushMatrix();
        glTranslated(u.first.x, (u.first.z + 0.5)*MeterPerEl, u.first.y + 0.5);
        Engine::glDrawArrow(Engine::Vector(u.second, 0, 0));
        glPopMatrix();
    }
    for (auto v : voxels.V())
    {
        glPushMatrix();
        glTranslated(v.first.x + 0.5, (v.first.z + 0.5)*MeterPerEl, v.first.y);
        Engine::glDrawArrow(Engine::Vector(0, 0, v.second));
        glPopMatrix();
    }
    for (auto w : voxels.W())
    {
        glPushMatrix();
        glTranslated(w.first.x + 0.5, w.first.z*MeterPerEl, w.first.y + 0.5);
        Engine::glDrawArrow(Engine::Vector(0, w.second, 0));
        glPopMatrix();
    }
    glColor3d(0.5, 0.5, 0.5);
    for (auto d : Direction::all)
    {
        for (auto u : voxels.U().BoundaryCondition(d))
        {
            double v = u.first;
            glPushMatrix();
            glTranslated(u.first.x, (u.first.z + 0.5)*MeterPerEl, u.first.y + 0.5);
            Engine::glDrawArrow(Engine::Vector(u.second, 0, 0));
            glPopMatrix();
        }
        for (auto v : voxels.V().BoundaryCondition(d))
        {
            glPushMatrix();
            glTranslated(v.first.x + 0.5, (v.first.z + 0.5)*MeterPerEl, v.first.y);
            Engine::glDrawArrow(Engine::Vector(0, 0, v.second));
            glPopMatrix();
        }
        for (auto w : voxels.W().BoundaryCondition(d))
        {
            glPushMatrix();
            glTranslated(w.first.x + 0.5, w.first.z*MeterPerEl, w.first.y + 0.5);
            Engine::glDrawArrow(Engine::Vector(0, w.second, 0));
            glPopMatrix();
        }
    }

    for (auto d : Direction::all)
    {
        for (auto v : voxels.BoundaryCondition(Directions(d)))
        {
            glPushMatrix();
            glTranslated(v.position.x, v.position.z*MeterPerEl, v.position.y);
            v.Render(v.position, Directions(0xFFFF), Engine::RGBA::white);
            glPopMatrix();
        }
    }

    Engine::CheckGLError();
}

std::wistream& operator>>(std::wistream& s, VoxelMap& map)
{
    std::wstring name;
    unsigned longitude, latitude, altitude;
    s >> name >> longitude >> latitude >> altitude;
    map.Space(longitude, latitude, altitude);
    double temperature;
    int waterLevel;
    s >> temperature >> waterLevel;
    map.Water(waterLevel, temperature);
    map.Air(temperature, 20000);
    unsigned procedures;
    s >> procedures;
    for (unsigned p = 0; p < procedures; ++p)
    {
        std::wstring procedure;
        s >> procedure;
        if (procedure == L"HILL")
        {
            Engine::Coordinate p0, p1;
            s >> p0 >> p1;
            float stddev;
            s >> stddev;
            map.Hill(p0, p1, stddev);
        }
        else
        {
            throw std::runtime_error("Unknown procedure");
        }
    }
    return s;
}

}