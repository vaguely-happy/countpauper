#include "pch.h"
#include "Parser/Parser.h"
#include "Logic/Predicate.h"
#include "Logic/Sequence.h"
#include "Logic/Boolean.h"
#include "Logic/Integer.h"

namespace Angel
{
namespace Parser
{
namespace Test
{

    /* Reenable with BNF

TEST(TestElement, Id)
{
    std::wstringstream s("cat");
    Logic::Object cat;
    s >> cat;
    EXPECT_EQ(cat, Logic::id("cat"));
}

TEST(TestElement, Ids)
{
    std::wstringstream s("gizmo ginny");
    Logic::Object gizmo, ginny;
    s >> gizmo >> ginny;
    EXPECT_EQ(gizmo, Logic::id("gizmo"));
	EXPECT_EQ(ginny, Logic::id("ginny"));
}

TEST(TestElement, Boolean)
{
    std::wstringstream s("true false");
    Logic::Object yes, no;
    s >> yes >> no;

    EXPECT_EQ(yes, Logic::boolean(true));
    EXPECT_EQ(no, Logic::boolean(false));
}

TEST(TestElement, Integer)
{
    std::wstringstream s("-9 23 cat2");
    Logic::Object negative, positive, id;
    s >> negative >> positive >> id;

    EXPECT_EQ(positive, Logic::integer(23));
    EXPECT_EQ(negative, Logic::integer(-9));
    EXPECT_EQ(id, Logic::id("cat2"));
}
*/



}
}
}

