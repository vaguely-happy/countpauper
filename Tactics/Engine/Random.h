#pragma once
#include <random>

namespace Engine
{
    class Generator
    {
    public:
        Generator();
        Generator(unsigned seed);
        double Chance();
        double Normal(double sigma); 
        using Seed = unsigned;
        Seed GetSeed() const;
        void SetSeed(Seed newSeed);
        Seed Store();
        void Restore(Seed);
    private:
        Seed seed;
        std::mt19937 generator;
    };
    Generator& Random();
}