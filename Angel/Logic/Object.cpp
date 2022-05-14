#include "stdafx.h"
#include "Object.h"
#include "Boolean.h"
#include "Integer.h"
#include "Id.h"

namespace Angel
{
namespace Logic
{

Object::Object(const std::wstring& tag)
{
	if (tag.empty())
	{
		return;
	}
	else if (auto boolean = Boolean::Parse(tag))
	{
        expr = std::make_unique<Boolean>(*boolean);
	}
	else if (auto integer = Integer::Parse(tag))
	{
        expr = std::make_unique<Integer>(*integer);
	}
	else
	{
        expr = std::make_unique<Id>(tag);
	}

}
Object::Object(Object&& other) :
    expr(std::move(other.expr))
{
}

Object& Object::operator=(Object&& other)
{
    expr = std::move(other.expr);
	return *this;
}

Object::operator bool() const
{
	return expr.get();
}

bool Object::Trivial() const
{
	return operator==(boolean(true));
}


bool Object::operator==(const Object& other) const
{
	return *expr == *other.expr;
}

bool Object::Match(const Expression& other, const Knowledge& knowledge) const
{
	return expr->Match(other, knowledge);
}

Object Object::Compute(const Knowledge& knowledge) const
{
    return expr->Compute(knowledge);
}

const Expression& Object::operator*() const
{
    return *expr;
}

size_t Object::Hash() const
{
	return reinterpret_cast<size_t>(expr.get());
}

Object Object::Cast(const std::type_info& t, const Knowledge& knowledge) const
{
    return expr->Cast(t, knowledge);
}



}
}