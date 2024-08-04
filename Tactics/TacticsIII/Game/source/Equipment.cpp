#include "Game/Equipment.h"

namespace Game
{

Equipment::Equipment(const Item& item) :
    Counters(item),
    item(&item)
{
}

Equipment::Equipment(const Equipment& o) :
    Counters(o),
    item(o.item)
{
}

Equipment::Equipment(Equipment&& o) :
    Counters(std::move(o)),
    item(o.item)
{
}

Equipment& Equipment::operator=(const Equipment& o)
{
    item = o.item;
    Counters::operator=(o);
    return *this;
}

Equipment& Equipment::operator=(Equipment&& o)
{
    item = nullptr;
    std::swap(item, o.item);
    return *this;
}

StatDescriptor Equipment::Get(Stat::Id id) const
{
    return item->Get(id);
}

const Item& Equipment::GetItem() const
{
    return *item;
}

}
