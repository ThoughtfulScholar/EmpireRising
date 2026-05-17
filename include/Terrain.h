#ifndef TERRAIN_H
#define TERRAIN_H

#include "Enums.h"
#include <vector>
#include <raylib.h>

class WorldMap {
private:
    int width;
    int height;
    std::vector<std::vector<TerrainType>> grid;

public:
    WorldMap(int w, int h);

    void setTile(int x, int y, TerrainType type);
    [[nodiscard]] TerrainType getTile(int x, int y) const;
    
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;

    // Algoritmul de trasare drumuri garantate între două puncte coordinate
    void createPath(int startX, int startY, int endX, int endY);
    
    // Validarea mișcărilor în funcție de limitele hărții și tipul de teren
    [[nodiscard]] bool isValidMove(int x, int y) const;

    // Randarea hărții folosind biblioteca grafică raylib
    void draw(int offsetX, int offsetY) const;
};

#endif // TERRAIN_H