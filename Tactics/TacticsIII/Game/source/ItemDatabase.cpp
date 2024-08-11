#include "Game/ItemDatabase.h"
#include "Utility/String.h"
#include <nlohmann/json.hpp>
#include "File/Json.h"
#include <iostream>
#include <fstream>

namespace Game
{
using json = nlohmann::json;

std::vector<Item> Parse(const json& data, std::string_view section)
{
    std::vector<Item> result;
    const auto& sectionData = Engine::must_get<json>(data, section);
    for(const auto& itemData : sectionData)
    {
        result.emplace_back(itemData);
    }
    return result;
}

ItemDatabase::ItemDatabase(const json& data)
{
    auto weapons = Parse(data, "weapon");
    items = weapons;
    auto armors = Parse(data, "armor");
    items.insert(items.end(), armors.begin(), armors.end());
}

const Item* ItemDatabase::Find(std::string_view name) const
{
    for(const auto& item : items)
    {
        if (item.Name() == name)
            return &item;
    }
    return nullptr;
}

}
