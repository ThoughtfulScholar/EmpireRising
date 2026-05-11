#ifndef WORLD_MAP_HPP
#define WORLD_MAP_HPP

#include "GlobalEnums.hpp"
#include <vector>
#include <iostream>
#include <raylib-cpp.hpp>

class Tile {
private:
    TerrainType type;
    bool walkable;
public:
    explicit Tile(TerrainType t = TerrainType::PLAIN);
    [[nodiscard]] TerrainType getType() const { return type; }
    [[nodiscard]] bool isWalkable() const { return walkable; }
    void setType(TerrainType t);
};

class WorldMap {
private:
    int width, height;
    std::vector<std::vector<Tile>> grid;

public:
    WorldMap(int w, int h);
    
    void createPath(int x1, int y1, int x2, int y2);
    void setTile(int x, int y, TerrainType t);
    [[nodiscard]] bool isValidMove(int x, int y) const;
    
    void draw(int offX, int offY) const;

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }

    friend std::ostream& operator<<(std::ostream& os, const WorldMap& wm);
};

#endif
