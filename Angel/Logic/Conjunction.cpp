#include "stdafx.h"
#include "Conjunction.h"
#include "Boolean.h"
#include "Knowledge.h"

namespace Angel
{
namespace Logic
{

Conjunction::Conjunction(Array&& operands) :
    Nary(std::move(operands))
{
}

Conjunction::Conjunction(Conjunction&& value) :
    Conjunction(std::move(value.operands))
{
}


bool Conjunction::operator==(const Expression& other) const
{
    // TODO: could be in Nary if the type was the same 
    if (auto conjunction = dynamic_cast<const Conjunction*>(&other))
    {
        return operands == conjunction->operands;
    }
    return false;
}


bool Conjunction::Match(const Expression& other, const Knowledge& knowledge) const
{
    for (const auto& condition : operands)
    {
        if (!knowledge.Query(condition))
            return false;
    }
    return true;
}

Object Conjunction::Compute(const Knowledge& knowledge) const
{
    for (const auto& condition : operands)
    {
        auto truth = condition.Compute(knowledge);
        if (!truth.Trivial())
            return truth;
        // TODO: compute the operands, but cast them to boolean, forcing clauses to be queried? 
        if (!knowledge.Query(condition))
            return boolean(false);
    }
    return boolean(true);
}

Object Conjunction::Cast(const std::type_info& t, const Knowledge& k) const
{
    if (t == typeid(Boolean))
    {
        return Compute(k);
    }
    throw CastException<Conjunction>(t);
}


}
}