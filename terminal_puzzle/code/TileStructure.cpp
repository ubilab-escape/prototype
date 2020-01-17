// Copyright 2019
// Author: Lukas König

#include "./TileStructure.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "./Tile.h"

// global variables
std::vector<std::vector<Tile>> _tiles;
std::vector<std::vector<Tile>> _initial_tiles;

// Constructor 1: Initialize TileStructure by 2D Tile vector.
TileStructure::TileStructure(std::vector<std::vector<Tile>> tiles) {
    _tiles = tiles;
    _initial_tiles = tiles;
}

// Constructor 2: Initialize TileStructure by 2D pixel vector.
TileStructure::TileStructure(std::vector<std::vector<int>> pixels) {
    this->pixelsToTiles(pixels);
    _initial_tiles = _tiles;
}

// Constructor 3: Initialize TileStructure using file.
TileStructure::TileStructure(const char* filepath) {
    this->pixelsFromFile(filepath);
    _initial_tiles = _tiles;
}

// function: changes _pixels to new 2D pixel array from 2D pixel vector.
void TileStructure::pixelsToTiles(std::vector<std::vector<int>> pixels) {
    int N = 4;
    int P = pixels.size() - pixels.size() % N;
    int T = P / N;
    Tile single_tile = Tile(T);
    std::vector<std::vector<Tile>> tiles(N, std::vector<Tile>(N, single_tile));
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < P; j++) {
            int tileI = i / T;
            int tileJ = j / T;
            int pixI = i % T;
            int pixJ = j % T;
            int type = (tileI % 2) + 2*(tileJ % 2);
            tiles[tileI][tileJ]._pixels[pixI][pixJ] = pixels[i][j];
            tiles[tileI][tileJ]._type = type;
        }
    }
    _tiles = tiles;
}


// function: changes _pixels to 2D Tile vector extracted from file.
void TileStructure::pixelsFromFile(const char* filepath) {
    std::ifstream in;
    in.open(filepath, std::ios::in);
    std::vector<std::vector<int>> pixels;
    std::string row;
    char c;
    while (in >> row) {
        std::vector<int> pixelline;
        for (unsigned int k = 0; k < row.length(); k++) {
            c = row[k];
            if (c == '0' || c == '1') {
                int pix = (c == '1');
                pixelline.push_back(pix);
            }
        }
        pixels.push_back(pixelline);
    }
    this->pixelsToTiles(pixels);
}

// print all the types of the TileStruct
void TileStructure::printTypes() {
    int N = _tiles.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            std::cout << _tiles[i][j]._type;
            std::cout << ", ";
        }
        std::cout << "\n";
    }
}

// function: Turns all the Tiles of a specific Tile Right
void TileStructure::turnTilesRight(int type) {
    int N = _tiles.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (_tiles[i][j]._type == type) {
                _tiles[i][j].turnPixelsRight();
            }
        }
        std::cout << "\n";
    }
}

// function: Turns all the Tiles in TileStruct around.
void TileStructure::turnTilesAround(int type) {
    int N = _tiles.size();
    bool last = false;
    Tile lastTile = Tile(_tiles[0][0]._pixels);
    Tile currentTile = Tile(_tiles[0][0]._pixels);
    int beginI = 0;
    int beginJ = 0;
    bool forward = true;
    bool tileinline = false;
    for (int i = 0; i < N; i++) {
        for (int j2 = 0, j = forward ? j2 : (N-1) - j2;
             j2 < N; j2++, j = forward ? j2 : (N-1) - j2) {
            if (_tiles[i][j]._type == type && last) {
                currentTile = _tiles[i][j];
                _tiles[i][j] = lastTile;
                lastTile = currentTile;
                tileinline = true;
            } else if (_tiles[i][j]._type == type && !last) {
                lastTile = _tiles[i][j];
                beginI = i;
                beginJ = j;
                last = true;
                tileinline = true;
            }
        }
        if (tileinline) {
            forward = !forward;
            tileinline = false;
        }
    }
    if (last) {
        _tiles[beginI][beginJ] = lastTile;
    }
}

// function: Draw the TileStructure using ncurses
void TileStructure::draw(int posX, int posY) {
    int N = _tiles.size();
    int T = _tiles[0][0]._pixels.size();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_BLUE, -1);
    init_pair(4, COLOR_YELLOW, -1);
    init_pair(5, COLOR_MAGENTA, -1);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            attron(COLOR_PAIR(_tiles[i][j]._type + 1));
            _tiles[i][j].draw(posX + 2*j*T, posY + i*T);
            attroff(COLOR_PAIR(_tiles[i][j]._type + 1));
        }
    }
}

// function: applies 20 random turn functions to TileStructure.
void TileStructure::shuffle() {
    int randtype = 0;
    int randop = 0;
    unsigned int seed = time(NULL);
    for (int c = 0; c < 20; c++) {
        randtype = rand_r(&seed) % 4;
        randop = rand_r(&seed) % 2;
        if (randop == 0) {
            this->turnTilesRight(randtype);
        } else if (randop == 1) {
            this->turnTilesAround(randtype);
        }
    }
}

// bool: checks if Tile equals its initial state.
bool TileStructure::solved() {
    return _tiles == _initial_tiles;
}

// function: change the type (and therefore color) of all the tiles.
void TileStructure::colorWhite() {
    int N = _tiles.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            _tiles[i][j]._type = 4;
        }
    }
}

// friend function: Use the output stream to print out TileStruct pixels.
std::ostream & operator << (std::ostream &out,
                            const TileStructure &tileStruct) {
    int N = tileStruct._tiles.size();
    int T = tileStruct._tiles[0][0]._pixels.size();
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < T; k++) {
            for (int j = 0; j < N; j++) {
                for (int l = 0; l < T; l++) {
                    out << tileStruct._tiles[i][j]._pixels[k][l];
                    out << ", ";
                }
                out << "  ";
            }
            out << "\n";
        }
        out << "\n";
    }
    return out;
}

// friend function: Use == to check equality of TileStructures
bool operator== (const TileStructure &tstruct1, const TileStructure &tstruct2) {
    return (tstruct1._tiles == tstruct2._tiles);
}

