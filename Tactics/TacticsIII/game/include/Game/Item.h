#pragma once
#include "Game/StatDefinition.h"
#include <map>
#include <string>

namespace Game
{

class Item
{
public:
        Item();
        static StatDefinition definition;
private:
        std::string name;
        std::map<Stat::Id, int> bonus;
        std::map<Stat::Id, int> enchantment;
};

}
