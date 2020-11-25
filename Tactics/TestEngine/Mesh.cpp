#include "pch.h"
#include "Mesh.h"
#include "AxisAlignedBoundingBox.h"
#include "OffscreenSurface.h"
#include "Matrix.h"
#include <gl/glew.h>

namespace Engine::Test
{

struct Mesh : public ::testing::Test
{
    Mesh() = default;
    OffscreenSurface surface;
    const double precision = 1e-12;
};

TEST_F(Mesh, BoundingBox)
{
    Box cube(1.0);
    EXPECT_NEAR(1.0, cube.GetBoundingBox().Volume(), precision);
    EXPECT_TRUE(cube.GetBoundingBox().Contains(Engine::Coordinate(0, 0, 0)));
}

TEST_F(Mesh, BoxVolume)
{   // Test IVolume interface with bounding box
    Box cube(2.0);
    EXPECT_NEAR(8.0, cube.GetBoundingBox().Volume(), precision);
    // and exact distance to surface
    Coordinate center(0, 0, 0);
    EXPECT_EQ(-1.0, cube.Distance(center));
    EXPECT_EQ(-0.5, cube.Distance(center + Vector(0.5, 0, 0)));
    EXPECT_EQ(-0.25, cube.Distance(center + Vector(0, 0.75, -0.75)));

    std::vector<Vector> dirs = { Vector(1,0,0),Vector(0,1,0), Vector(0,0,1), Vector(-1,0,0), Vector(0,-1,0), Vector(0,0,-1) };
    for (auto dir : dirs)
    {
        EXPECT_NEAR(0, cube.Distance(center+dir), precision) << dir;
        EXPECT_NEAR(1, cube.Distance(center + dir * 2.0), precision) << dir;
    }

}

TEST_F(Mesh, Render)
{
    Box cube(1.0);
    cube.Render();
    cube.Invalidate();
    cube.Render();
}

TEST_F(Mesh, Scale)
{
    Box cube(1.0);
    cube *= Matrix::Scale(Engine::Vector(2, 2, 2));
    EXPECT_NEAR(8.0, cube.GetBoundingBox().Volume(), 1e-6);
}

TEST_F(Mesh, Translate)
{
    Box cube(1.0);
    cube *= Matrix::Translation(Engine::Vector(1, 1, 1));
    EXPECT_FALSE(cube.GetBoundingBox().Contains(Engine::Coordinate(0, 0, 0)));
}

}
