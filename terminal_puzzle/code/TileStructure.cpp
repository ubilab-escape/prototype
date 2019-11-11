// Copyright 2019
// Author: Lukas König

#include "./TileStructure.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
#include "./Tile.h"

std::vector<std::vector<Tile>> _tiles;

TileStructure::TileStructure(std::vector<std::vector<Tile>> tiles) {
    _tiles = tiles;
}

TileStructure::TileStructure(std::vector<std::vector<bool>> pixels) {
    int P = pixels.size();
    int N = 4;
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

void TileStructure::turnTilesRight(int type) {
    int N = _tiles.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (_tiles[i][j]._type == type) {
                _tiles[i][j].turnRight();
            }
        }
        std::cout << "\n";
    }
}

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

void TileStructure::draw(int posX, int posY) {
    int N = _tiles.size();
    int T = _tiles[0][0]._pixels.size();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_BLUE, -1);
    init_pair(4, COLOR_YELLOW, -1);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            attron(COLOR_PAIR(_tiles[i][j]._type + 1));
            _tiles[i][j].draw(posX + 2*j*T, posY + i*T);
            attroff(COLOR_PAIR(_tiles[i][j]._type + 1));
        }
    }
}

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

bool operator== (const TileStructure &tstruct1, const TileStructure &tstruct2) {
    return (tstruct1._tiles == tstruct2._tiles);
}

