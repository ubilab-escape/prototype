// Copyright 2018
// Author: Lukas König

#ifndef TILESTRUCTURE_H_
#define TILESTRUCTURE_H_

#include <vector>
#include "./Tile.h"

class TileStructure {
 public:
    // constructor
    explicit TileStructure(std::vector<std::vector<Tile>> tiles);
    explicit TileStructure(std::vector<std::vector<int>> pixels);
    explicit TileStructure(const char* filepath);

    void pixelsFromFile(const char* filepath);
    void pixelsToTiles(std::vector<std::vector<int>> pixels);
    void turnTilesRight(int type);
    void turnTilesAround(int type);
    void printTypes();
    void printTiles();
    void shuffle();
    bool solved();
    void draw(int posX, int posY);
    void colorWhite();
    int size(); 
    friend std::ostream & operator<< (std::ostream &out,
                         const TileStructure &tileStruct);
    friend bool operator== (const TileStructure &tstruct1,
                            const TileStructure &tstruct2);
    std::vector<std::vector<Tile>> _tiles;
};
#endif  // TILESTRUCTURE_H_

