#pragma once

#include "Game/Equipped.h"

namespace Game
{

class Equipments : public Equipped
{
public:
    Equipments() = default;
    explicit Equipments(const class ItemDatabase& db, const json& data);

    virtual ~Equipments() = default;

    const Equipment& Equip(const Equipment& equipment) override;
    bool Unequip(const Equipment& item) override;
    unsigned Unequip(const Restrictions filter) override;
    std::vector<const Equipment*> GetEquipped(const Restrictions& filter={}) const override;
    Computation Get(Stat::Id id, const class Boni* extraBoni = nullptr, const Restrictions& filter={}) const override;
    json Serialize() const;
protected:
    std::vector<Equipment> equipped;
};

}
