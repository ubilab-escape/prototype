// Copyright 2019
// Author: Lukas König

#include <gtest/gtest.h>
#include <vector>
#include "./Tile.h"
#include "./TileStructure.h"

TEST(TileTest, TurnRightTest) {
    std::vector<std::vector<int>> pixels_0 ={{1, 0, 1}, {1, 1, 1}, {0, 0, 1}};
    std::vector<std::vector<int>> pixels_1 = {{0, 1, 1}, {0, 1, 0}, {1, 1, 1}};
    std::vector<std::vector<int>> pixels_2 = {{1, 0, 0}, {1, 1, 1}, {1, 0, 1}};
    std::vector<std::vector<int>> pixels_3 = {{1, 1, 1}, {0, 1, 0}, {1, 1, 0}};
    Tile tileA = Tile(pixels_0);
    tileA.turnPixelsRight();
    ASSERT_EQ(tileA._pixels, pixels_1);
    tileA.turnPixelsRight();
    ASSERT_EQ(tileA._pixels, pixels_2);
    tileA.turnPixelsRight();
    ASSERT_EQ(tileA._pixels, pixels_3);
    tileA.turnPixelsRight();
    ASSERT_EQ(tileA._pixels, pixels_0);
}

TEST(TileTest, TurnDirectionTest) {
    std::vector<std::vector<int>> pixels_0 = {{1, 0, 1}, {1, 1, 1}, {0, 0, 1}};
    std::vector<std::vector<int>> pixels_1 = {{0, 1, 1}, {0, 1, 0}, {1, 1, 1}};
    std::vector<std::vector<int>> pixels_2 = {{1, 0, 0}, {1, 1, 1}, {1, 0, 1}};
    std::vector<std::vector<int>> pixels_3 = {{1, 1, 1}, {0, 1, 0}, {1, 1, 0}};
    Tile tileA = Tile(pixels_0);
    tileA.turnDirection(3);
    ASSERT_EQ(tileA._pixels, pixels_3);
    tileA.turnDirection(1);
    ASSERT_EQ(tileA._pixels, pixels_1);
    tileA.turnDirection(4);
    ASSERT_EQ(tileA._pixels, pixels_0);
    tileA.turnDirection(0);
    ASSERT_EQ(tileA._pixels, pixels_0);
}

TEST(TileStructTest, TurnTypeTest) {
    std::vector<std::vector<int>> pixels_a0 = {{1, 0, 1},
                                                {1, 1, 1},
                                                {0, 0, 1}};
    std::vector<std::vector<int>> pixels_b0 = {{0, 1, 1},
                                                {0, 1, 0},
                                                {1, 1, 1}};
    std::vector<std::vector<int>> pixels_c0 = {{0, 1, 0},
                                                {0, 1, 0},
                                                {0, 0, 0}};

    Tile tileA = Tile(pixels_a0, 0);
    tileA.turnDirection(1);
    std::vector<std::vector<int>> pixels_a1 = tileA._pixels;
    tileA.turnDirection(2);
    std::vector<std::vector<int>> pixels_a2 = tileA._pixels;
    tileA.turnDirection(3);
    std::vector<std::vector<int>> pixels_a3 = tileA._pixels;
    tileA.turnDirection(0);

    Tile tileB = Tile(pixels_b0, 1);
    tileB.turnDirection(1);
    std::vector<std::vector<int>> pixels_b1 = tileB._pixels;
    tileB.turnDirection(2);
    std::vector<std::vector<int>> pixels_b2 = tileB._pixels;
    tileB.turnDirection(3);
    std::vector<std::vector<int>> pixels_b3 = tileB._pixels;
    tileB.turnDirection(0);

    Tile tileC = Tile(pixels_c0, 2);
    tileC.turnDirection(1);
    std::vector<std::vector<int>> pixels_c1 = tileC._pixels;
    tileC.turnDirection(2);
    std::vector<std::vector<int>> pixels_c2 = tileC._pixels;
    tileC.turnDirection(3);
    std::vector<std::vector<int>> pixels_c3 = tileC._pixels;
    tileC.turnDirection(0);

    std::vector<std::vector<Tile>> tiles = {{tileA, tileB}, {tileB, tileC}};
    TileStructure tstruct = TileStructure(tiles);

    ASSERT_EQ(tstruct._tiles[0][0]._pixels, pixels_a0);
    ASSERT_EQ(tstruct._tiles[0][1]._pixels, pixels_b0);
    ASSERT_EQ(tstruct._tiles[1][0]._pixels, pixels_b0);
    ASSERT_EQ(tstruct._tiles[1][1]._pixels, pixels_c0);

    tstruct.turnTilesRight(1);

    ASSERT_EQ(tstruct._tiles[0][0]._pixels, pixels_a0);
    ASSERT_EQ(tstruct._tiles[0][1]._pixels, pixels_b1);
    ASSERT_EQ(tstruct._tiles[1][0]._pixels, pixels_b1);
    ASSERT_EQ(tstruct._tiles[1][1]._pixels, pixels_c0);

    tstruct.turnTilesRight(0);
    tstruct.turnTilesRight(0);

    ASSERT_EQ(tstruct._tiles[0][0]._pixels, pixels_a2);
    ASSERT_EQ(tstruct._tiles[0][1]._pixels, pixels_b1);
    ASSERT_EQ(tstruct._tiles[1][0]._pixels, pixels_b1);
    ASSERT_EQ(tstruct._tiles[1][1]._pixels, pixels_c0);

    tstruct.turnTilesRight(0);
    tstruct.turnTilesRight(0);
    tstruct.turnTilesRight(2);
    tstruct.turnTilesRight(1);

    ASSERT_EQ(tstruct._tiles[0][0]._pixels, pixels_a0);
    ASSERT_EQ(tstruct._tiles[0][1]._pixels, pixels_b2);
    ASSERT_EQ(tstruct._tiles[1][0]._pixels, pixels_b2);
    ASSERT_EQ(tstruct._tiles[1][1]._pixels, pixels_c1);
}

TEST(TileStructTest, TurnTileAroundTest) {
    std::vector<std::vector<int>> pixels_0 = {{1, 0, 1}, {1, 1, 1}, {0, 0, 1}};
    std::vector<std::vector<int>> pixels_1 = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    std::vector<std::vector<int>> pixels_2 = {{1, 0, 0}, {1, 1, 1}, {1, 0, 1}};
    std::vector<std::vector<int>> pixels_3 = {{1, 1, 1}, {0, 1, 0}, {1, 1, 0}};
    Tile tileA = Tile(pixels_0, 0);
    Tile tileB = Tile(pixels_1, 0);
    Tile tileC = Tile(pixels_2, 1);
    Tile tileD = Tile(pixels_3, 0);
    std::vector<std::vector<Tile>> tiles0 = {{tileA, tileB}, {tileC, tileD}};
    std::vector<std::vector<Tile>> tiles1 = {{tileD, tileA}, {tileC, tileB}};
    std::vector<std::vector<Tile>> tiles2 = {{tileC, tileD}, {tileB, tileA}};
    TileStructure tstruct = TileStructure(tiles0);
    ASSERT_EQ(tstruct._tiles, tiles0);
    tstruct.turnTilesAround(0);
    ASSERT_EQ(tstruct._tiles, tiles1);
    tstruct._tiles[1][0]._type = 0;
    tstruct.turnTilesAround(0);
    ASSERT_EQ(tstruct._tiles, tiles2);
    tstruct._tiles[0][0]._type = 1;
    tstruct._tiles[0][1]._type = 2;
    tstruct._tiles[1][0]._type = 3;
    tileA.turnDirection(0);
    ASSERT_EQ(tstruct._tiles, tiles2);
}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
