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

Tile::Tile(int N, int type) {
     std::vector<std::vector<int>> pixels(N , std::vector<int> (N, 0));
    _pixels = pixels;
    _type = type;
}

Tile::Tile(std::vector<std::vector<int>> pixels, int type) {
    _pixels = pixels;
    _direction = 0;
    _type = type;
}

void Tile::turnDirection(int direction) {
    int n = this->real_mod(direction - _direction, 4);
    for (int i = 0; i < n; i++) {
        this->turnPixelsRight();
    }
    _direction = direction % 4;
}

void Tile::turnRight() {
    _direction = (_direction + 1) % 4;
    this->turnPixelsRight();
}

void Tile::turnPixelsRight() {
    std::vector<std::vector<int>> new_pixels = _pixels;
    int N = new_pixels.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            new_pixels[i][j] = _pixels[(N-1)-j][i];
        }
    }
    _pixels = new_pixels;
}

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

bool operator== (const Tile &tile1, const Tile &tile2) {
    return (tile1._pixels == tile2._pixels);
}

int Tile::real_mod(int a, int b) {
    return (b + (a % b)) % b;
}
