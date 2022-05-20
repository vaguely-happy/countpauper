#include "pch.h"
#include "Logic/Knowledge.h"
#include "Logic/Conjunction.h"
#include "Logic/Boolean.h"
#include "Logic/Predicate.h"

namespace Angel::Logic::Test
{

TEST(TestComputation, Conjunctions)
{
    Knowledge k;
    // TODO: not all expression should be knowable. Elements and operators are not the same as predicates, clauses and ... formulas? 
    EXPECT_TRUE(conjunction().Compute(k).Trivial());
    EXPECT_TRUE(conjunction(boolean(true)).Compute(k).Trivial());
    EXPECT_FALSE(conjunction(boolean(true), boolean(false)).Compute(k).Trivial());
    EXPECT_TRUE(conjunction(conjunction(boolean(true))).Compute(k).Trivial());
    EXPECT_FALSE(conjunction(conjunction(boolean(false)), conjunction(boolean(true))).Compute(k).Trivial());
    EXPECT_FALSE(conjunction(predicate(L"cat")).Trivial());
    k.Know(predicate(L"cat", Sequence()));
    EXPECT_TRUE(conjunction(predicate(L"cat")).Compute(k).Trivial());
}

}

