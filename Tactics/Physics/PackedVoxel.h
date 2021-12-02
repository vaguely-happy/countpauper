#include "Physics/Material.h"
#include "Engine/Color.h"
namespace Physics
{
#pragma pack(push)
#pragma pack(2)

    class PackedVoxel
    {
    public:
        static const int maxAmount = 15;
        PackedVoxel();
        // TODO: amount could be double density on interface as well, but it's much slower to fill large areas and compute over an over again
        // and the discrete physics is not that continuous either
        PackedVoxel(const Material& m, double temperature, int amount = maxAmount);

        void Set(const Material* newMat, unsigned  amount = maxAmount);
        void Set(const Material& newMat, unsigned  amount = maxAmount) { Set(&newMat, amount); }
        const Material* GetMaterial() const;
        Engine::RGBA Color() const;
        double Temperature() const;
        void SetTemperature(double t);
        double Density() const;
        int Amount() const;
    private:

        // material: 0 = vacuum, air, water, earth, stone
        // amount 0 = empty, 15 = full/max density or granularity
        // hot = 0: temperature = 2 * k (0-510), 1 = 510 + 30 k (510-1275) 
        uint8_t material : 2, :1, amount : 4, hot : 1, temperature : 8;

        static const int ColdTGradient = 2;
        static const int HotTGradient = 30;
        static const int HotOffset = 255 * ColdTGradient;
        static const Material* mats[];
    };
#pragma pack(pop)
}