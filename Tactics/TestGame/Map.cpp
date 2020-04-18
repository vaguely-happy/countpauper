#include "pch.h"
#include <numeric>
#include "Game/Map.h"
#include "Data.h"

namespace Game
{
namespace Test
{

TEST(Map, At)
{
    Data::MiniMap map;
    EXPECT_EQ(Floor::Earth, map.At(Position(0, 0)).floor);
    EXPECT_EQ(1, map.At(Position(0, 0)).height);
    EXPECT_TRUE(map.CanBe(Position(0, 0)));
    EXPECT_EQ(Floor::Water, map.At(Position(1, 0)).floor);
    EXPECT_EQ(0, map.At(Position(1, 0)).height);
    EXPECT_FALSE(map.CanBe(Position(1, 0)));
}

TEST(Map, CanBe)
{
    Data::MiniMap map;
    EXPECT_TRUE(map.CanBe(Position(0, 0)));
    EXPECT_FALSE(map.CanBe(Position(1, 0)));
}

TEST(Map, MaybeAt)
{
    Data::MiniMap map;
    EXPECT_EQ(Floor::Earth, map.At(Position(0, 0)).floor);
    EXPECT_EQ(1, map.At(Position(0, 0)).height);

    EXPECT_FALSE(map.At(Position(-1, 0)));
    EXPECT_FALSE(map.At(Position(0, -1)));
    EXPECT_FALSE(map.At(Position(2, 0)));
    EXPECT_FALSE(map.At(Position(0, 2)));
}


TEST(Map, CanGo)
{
    Data::MiniMap map;
    EXPECT_FALSE(map.CanGo(Position(0, 0), Direction::none));
    EXPECT_TRUE(map.CanGo(Position(0, 0), Direction::north));
    EXPECT_FALSE(map.CanGo(Position(0, 0), Direction::south));
    // undefined if you can go to a square you can't be at EXPECT_FALSE(map.CanGo(Position(0, 0), Direction::east));
    EXPECT_FALSE(map.CanGo(Position(0, 0), Direction::west));

    EXPECT_FALSE(map.CanGo(Position(0, 1), Direction::north));
    EXPECT_FALSE(map.CanGo(Position(0, 1), Direction::east));
}

}
}  // Test