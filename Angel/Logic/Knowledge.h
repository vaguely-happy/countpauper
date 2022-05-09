#pragma once
#include "Namespace.h"
#include "Object.h"

namespace Angel
{
namespace Logic
{

// Knowledge is a set of items organized in namespaces
class Knowledge
{
public:
	Knowledge();
	void Know(Object&& e);
	bool Query(const Object& e) const;
	bool Knows(const Object& e) const;
	size_t Clauses() const;
private:
	Namespace root;
};


}
}