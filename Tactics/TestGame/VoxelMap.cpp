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
    EXPECT_FALSE(map.At(Position(0, 0, -1)));    
    EXPECT_EQ(0, map.Mass());
}

TEST(Map, Air)
{
    VoxelMap map;
    map.Space(2, 2, 2);
    map.Air(300, 20000);
    EXPECT_EQ(Element::Air, map.At(Position(0, 0, 0)).floor);
    EXPECT_FALSE(map.At(Position(0, 0, 0)).solid);
    EXPECT_EQ(Element::Air, map.At(Position(1, 1, 1)).floor);
    EXPECT_EQ(0, map.At(Position(1, 1, 1)).height);
    EXPECT_NEAR(1.17 * map.Volume(), map.Mass(),map.Volume()*0.01);
    EXPECT_NEAR(300, map.Temperature(VoxelMap::Material::air), 0.01);
}

TEST(Map, Lava)
{
    VoxelMap map;
    map.Space(2, 2, 2);
    map.World(10e6);

    EXPECT_EQ(Element::Fire, map.At(Position(0, 0, 0)).floor);
    EXPECT_FALSE(map.At(Position(0, 0, 0)).solid);
    EXPECT_EQ(Element::Fire, map.At(Position(1, 1, 1)).floor);
    EXPECT_EQ(0, map.At(Position(1, 1, 1)).height);
}

TEST(Map, Water)
{
    VoxelMap map;
    map.Space(2, 2, 2);
    map.Water(1, 300);

    EXPECT_EQ(Element::Water, map.At(Position(0, 0, 0)).floor);
    EXPECT_FALSE(map.At(Position(0, 0, 0)).solid);
    EXPECT_EQ(Element::Water, map.At(Position(1, 1, 1)).floor);
    EXPECT_EQ(0, map.At(Position(1, 1, 2)).height);
}

TEST(Map, HillPeak)
{
    VoxelMap map;
    map.Space(3, 3, 3);
    map.Air(300, 10000);
    map.Hill(Engine::Line(Engine::Coordinate(1.5, 1.5, 2), Engine::Coordinate(1.5, 1.5, 2)), 1.0f/float(Engine::FullWidthHalfMaximum(1)));

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
    map.World(10e6);
    map.Hill(Engine::Line(Engine::Coordinate(3, 0, 0), Engine::Coordinate(0, 3, 3)), 1.666 );
    // The line goes straight through the middle of (0,2) (1,1) and (2,0)
    // but is highest at the first and lowest at the last
    EXPECT_EQ(Element::Stone, map.At(Position(2, 0, 3)).floor);
    EXPECT_EQ(1, map.At(Position(2, 0, 3)).height);

    EXPECT_EQ(2, map.At(Position(1, 1, 2)).height);
    EXPECT_EQ(Element::Stone, map.At(Position(1, 1, 1)).floor);

    EXPECT_EQ(Element::Stone, map.At(Position(0, 2, 3)).floor);
    EXPECT_EQ(3, map.At(Position(0, 2, 3)).height);

    // 0,0 is one diagonal (roughly grid off the middle of the ridge line)
    //  which is close to full width half maximum of 1.666, so it should be one lower 
    EXPECT_EQ(Element::Stone, map.At(Position(0, 0, 3)).floor);
    EXPECT_EQ(1, map.At(Position(0, 0, 3)).height);


}


TEST(Map, StraightWall)
{
    VoxelMap map;
    map.Space(3, 3, 3);
    map.Wall(Engine::Line(Engine::Coordinate(1.5, -1, 0), Engine::Coordinate(1.5, 4, 0)), 2.0, 1.0);

    // Check along the wall
    EXPECT_EQ(Element::Stone, map.At(Position(1, 0, 3)).floor);
    EXPECT_EQ(2, map.At(Position(1, 0, 3)).height);
    EXPECT_EQ(Element::Stone, map.At(Position(1, 1, 3)).floor);
    EXPECT_EQ(2, map.At(Position(1, 1, 3)).height);
    EXPECT_EQ(Element::Stone, map.At(Position(1, 2, 3)).floor);
    EXPECT_EQ(2, map.At(Position(1, 2, 3)).height);

    // Next to the wall
    EXPECT_EQ(Element::None, map.At(Position(0, 0, 3)).floor);
    EXPECT_EQ(0, map.At(Position(0, 0, 3)).height);
    EXPECT_EQ(Element::None, map.At(Position(0, 0, 3)).floor);
    EXPECT_EQ(0, map.At(Position(2, 2, 3)).height);
}


}
}