#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <vector>
#include <iostream>
#include "Enums.h" // Necesar pentru TerrainType
#include "Exceptions.h"
#include <raylib.h>

class Tile {
private:
    TerrainType type;
    bool walkable;

public:
    explicit Tile(TerrainType t = TerrainType::PLAIN) : type(t) {
        walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
    }

    [[nodiscard]] TerrainType getType() const { return type; }
    [[nodiscard]] bool isWalkable() const { return walkable; }

    void setType(TerrainType t) {
        type = t;
        walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
    }
};

class WorldMap {
private:
    int width, height;
    std::vector<std::vector<Tile>> grid;

public:
    WorldMap(int w, int h);

    void createPath(int x1, int y1, int x2, int y2);
    void setTile(int x, int y, TerrainType t);

    [[nodiscard]] TerrainType getTile(int x, int y) const;
    [[nodiscard]] bool isValidMove(int x, int y) const;
    void draw(int offX, int offY) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Operator friend declarer
    friend std::ostream& operator<<(std::ostream& os, const WorldMap& wm);
};

#endif // WORLDMAP_H