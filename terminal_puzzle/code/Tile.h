// Copyright 2018
// Author: Lukas König

#ifndef TILE_H_
#define TILE_H_

#include <vector>
#include <iostream>

class Tile {
 public:
    // constructor
    explicit Tile(int size = 0, int type = 0);
    explicit Tile(std::vector<std::vector<int>> pixels, int type = 0);
    void turnDirection(int direction);
    void turnRight();
    void turnPixelsRight();
    void printPixels();
    void draw(int posX, int posY);
    int _direction;
    std::vector<std::vector<int>> _pixels;
    int _type;
    friend std::ostream & operator<< (std::ostream &out, const Tile &tile);
    friend bool operator== (const Tile &tile1, const Tile &tile2);
 private:
    int real_mod(int a, int b);
};
#endif  // TILE_H_
