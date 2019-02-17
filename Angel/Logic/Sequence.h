#pragma once
#include <vector>
#include "Id.h"
#include "Element.h"

namespace Angel
{
namespace Logic
{

class Array;

class Sequence : public Value, public std::vector<Element>
{
public:
	Sequence();
	explicit Sequence(Array&& array);
	explicit Sequence(Element&& value);
	template<class ...Args>
	explicit Sequence(Element&& first, Args... args) :
		Sequence(std::move(first))
	{
		Merge(Sequence(std::forward<Args>(args)...));
	}

//	Sequence(const std::initializer_list<Element&&>& init);
	Sequence(const Sequence&) = delete;
	Sequence& operator=(const Sequence&) = delete;
	Sequence(Sequence&& other);

	bool operator==(const Value& other) const override;
	bool Match(const Value& other, const Knowledge& knowledge) const override;
	void Append(Element&& value);
	void Merge(Sequence&& other);
};

Element sequence();
Element sequence(Array&& array);

template <bool...> struct bool_pack;
template <bool... v>
using all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;

template<class ...Args, class = std::enable_if_t <
	all_true < std::is_convertible<Args, Element>{}... > {}
>>
Element sequence(Args... args)
{
	return Element(std::make_unique<Sequence>(std::forward<Args>(args)...));
}

}
}
