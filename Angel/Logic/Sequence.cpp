#include "stdafx.h"
#include "Sequence.h"
#include "Array.h"

namespace Angel
{
namespace Logic
{

Sequence::Sequence() 
{
}

Sequence::Sequence(Array&& array)
{
	for (auto& e : array)
	{
		emplace_back(std::move(e));
	}
}

Sequence::Sequence(Object&& value)
{
	if (auto array = value.As<Array>())
	{
		for (auto& e : *array)
		{
			emplace_back(std::move(e));
		}
	}
	else if (value)
	{
		emplace_back(std::move(value));
	}
}


/* 	// Elements have to be copyable or no initializer list ... 
Sequence::Sequence(const std::initializer_list<Object&&>& init) 
{
	for (auto it = init.begin(); it!=init.end(); ++it)
	{
		contents.emplace_back(std::move(*it));
	}
}
*/

Sequence::Sequence(Sequence&& other) :
	std::vector<Object>(std::move(other))
{
}


bool Sequence::operator==(const Expression& other) const
{
	if (auto sequence= dynamic_cast<const Sequence*>(&other))
	{
		if (size() != sequence->size())
			return false;
		auto valueIt = sequence->begin();
		for (auto it = begin(); it!=end(); ++it, ++valueIt)
		{
			if (it->operator!=(*valueIt))
				return false;
		}
		return true;	
	}
	return false;
}

bool Sequence::Match(const Expression& expr, const Knowledge& knowledge) const
{
	if (auto sequence = dynamic_cast<const Sequence*>(&expr))
	{
		if (size() != sequence->size())
			return false;
		auto it = sequence->begin();
		for (const auto& e : *this)
		{
			if (!e.Match(**it, knowledge))
				return false;
			++it;	// TODO: zip
		}
		return true;
	}
	return false;
}

Object Sequence::Cast(const std::type_info& t, const Knowledge& k) const
{
    // TODO: array, set
    throw CastException<Sequence>(t);
}


void Sequence::Append(Object&& value)
{
	if (value)
		emplace_back(std::move(value));
}

void Sequence::Merge(Sequence&& other)
{
	for (auto& e : other)
		emplace_back(std::move(e));
}


Object sequence()
{
	return Object(std::make_unique<Sequence>());
}


Object sequence(Array&& array)
{
	return Create<Sequence>(std::move(array));
}

/*
Object sequence(const std::initializer_list<Object&&>& init)
{
	return Object<Sequence>(init);
}
*/


}
}