#include "pch.h"
#include <numeric>
#include "Game/VoxelMap.h"
#include "Engine/Maths.h"
#include "Data.h"

namespace Game
{
namespace Test
{

TEST(Map, Space)
{
    VoxelMap map;
    map.Space(1, 1, 1);
    EXPECT_FALSE(map.At(Position(0, 0, 0)));
    EXPECT_FALSE(map.At(Position(1, 0, 0)));
    EXPECT_FALSE(map.At(Position(0, -1, 0)));
    EXPECT_TRUE(map.At(Position(0, 0, -1)));    // lava
}

TEST(Map, Air)
{
    VoxelMap map;
    map.Space(2, 2, 2);
    map.Air(300, 20000);
    EXPECT_EQ(Element::Fire, map.At(Position(0, 0, 0)).floor);
    EXPECT_FALSE(map.At(Position(0, 0, 0)).solid);
    EXPECT_EQ(Element::Fire, map.At(Position(1, 1, 1)).floor);
    EXPECT_EQ(0, map.At(Position(1, 1, 1)).height);
}

TEST(Map, Water)
{
    VoxelMap map;
    map.Space(2, 2, 2);
    map.Air(300, 10000);
    map.Water(1, 300);

    EXPECT_EQ(Element::Water, map.At(Position(0, 0, 0)).floor);
    EXPECT_FALSE(map.At(Position(0, 0, 0)).solid);
    EXPECT_EQ(Element::Water, map.At(Position(1, 1, 1)).floor);
    EXPECT_EQ(1, map.At(Position(1, 1, 2)).height);
}

TEST(Map, HillPeak)
{
    VoxelMap map;
    map.Space(3, 3, 3);
    map.Air(300, 10000);
    map.Hill(Engine::Coordinate(1, 1, 2), Engine::Coordinate(1, 1, 2), 1.0f/ Engine::FullWidthHalfMaximum(1));

    EXPECT_EQ(Element::Stone, map.At(Position(1, 1, 2)).floor);
    EXPECT_EQ(2, map.At(Position(1, 1, 3)).height);
    EXPECT_EQ(Element::Stone, map.At(Position(1, 0, 2)).floor);
    EXPECT_EQ(1, map.At(Position(1, 0, 3)).height);
}

TEST(Map, HillRidge)
{
    VoxelMap map;
    map.Space(3, 3, 3);
    map.Air(300, 10000);
    map.Hill(Engine::Coordinate(2, 0, 0), Engine::Coordinate(0, 2, 2), 1.0 );

    EXPECT_EQ(Element::Fire, map.At(Position(2, 0, 3)).floor);
    EXPECT_EQ(0, map.At(Position(2, 0, 3)).height);

    EXPECT_EQ(1, map.At(Position(1, 1, 3)).height);
    EXPECT_EQ(Element::Stone, map.At(Position(1, 1, 1)).floor);

    EXPECT_EQ(Element::Stone, map.At(Position(0, 2, 3)).floor);
    EXPECT_EQ(2, map.At(Position(0, 2, 3)).height);
}
}
}