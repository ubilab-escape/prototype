// Copyright 2019
// Author: Lukas König

#include "./Tile.h"
#include <ncurses.h>
#include <vector>
#include <iostream>


// global variables
std::vector<std::vector<int>> _pixels;
int _direction;
int _type;

// Instructor 1: creates Tile of size NxN filled with 0.
// Type can be specified.
Tile::Tile(int N, int type) {
     std::vector<std::vector<int>> pixels(N , std::vector<int> (N, 0));
    _pixels = pixels;
    _type = type;
}

// Instructor 2: creates Tile from 2D pixel vector.
// Type can be specified.
Tile::Tile(std::vector<std::vector<int>> pixels, int type) {
    _pixels = pixels;
    _direction = 0;
    _type = type;
}


// function: Turns the 2D pixel vector of the Tile right.
void Tile::turnPixelsRight() {
    std::vector<std::vector<int>> new_pixels = _pixels;
    int N = new_pixels.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            new_pixels[i][j] = _pixels[(N-1)-j][i];
        }
    }
    _pixels = new_pixels;
    _direction = (_direction + 1) % 4;
}

// function: Turns the 2D pixel vector of Tile in a certian direction
// 0 -> Initial
// 1 -> Right
// 2 -> Upside Down
// 3 -> Left
void Tile::turnDirection(int direction) {
    int n = real_mod(direction - _direction, 4);
    for (int i = 0; i < n; i++) {
        this->turnPixelsRight();
    }
}

// function: Use ncurses to draw the 2D pixel vector of the Tile
void Tile::draw(int posX, int posY) {
    int N = this->_pixels.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (this->_pixels[i][j]) {
                attron(A_REVERSE);
            } else {
                attroff(A_REVERSE);
            }
            mvprintw(posY + i, posX + 2*j, "  ");
        }
    }
}

// friend function: make output stream possible for tile
std::ostream & operator << (std::ostream &out, const Tile &tile) {
    int N = tile._pixels.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            out << tile._pixels[i][j];
            out << ", ";
        }
        out << "\n";
    }
    return out;
}

// friend function: be able to check equality of Tiles using ==
bool operator== (const Tile &tile1, const Tile &tile2) {
    return (tile1._pixels == tile2._pixels);
}

// function: calculate real modulo.
// (C++ just calculates remainder using %)
int Tile::real_mod(int a, int b) {
    return (b + (a % b)) % b;
}
