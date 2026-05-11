#include "../include/WorldMap.hpp"

// --- IMPLEMENTARE TILE ---
Tile::Tile(TerrainType t) : type(t) {
    walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
}

void Tile::setType(TerrainType t) {
    type = t;
    walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
}

// --- IMPLEMENTARE WORLDMAP ---
WorldMap::WorldMap(int w, int h) : width(w), height(h) {
    grid.resize(height, std::vector<Tile>(width, Tile(TerrainType::PLAIN)));
}

void WorldMap::createPath(int x1, int y1, int x2, int y2) {
    int curX = x1;
    int curY = y1;
    while (curX != x2 || curY != y2) {
        if (curX < x2) curX++;
        else if (curX > x2) curX--;
        else if (curY < y2) curY++;
        else if (curY > y2) curY--;

        if (grid[curY][curX].getType() != TerrainType::CITY_TILE) {
            grid[curY][curX].setType(TerrainType::PLAIN);
        }
    }
}

void WorldMap::setTile(int x, int y, TerrainType t) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x].setType(t);
    }
}

bool WorldMap::isValidMove(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    return grid[y][x].isWalkable();
}

void WorldMap::draw(int offX, int offY) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            raylib::Color tileColor;
            switch (grid[y][x].getType()) {
                case TerrainType::FOREST:    tileColor = raylib::Color(34, 139, 34, 255); break;
                case TerrainType::MOUNTAIN:  tileColor = raylib::Color(105, 105, 105, 255); break;
                case TerrainType::WATER:     tileColor = raylib::Color(0, 191, 255, 255); break;
                case TerrainType::CITY_TILE: tileColor = raylib::Color(80, 80, 90, 255); break;
                default:                     tileColor = raylib::Color(100, 150, 70, 255); break;
            }
            DrawRectangle(offX + x * 40, offY + y * 40, 39, 39, tileColor);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const WorldMap& wm) {
    os << "Harta Lumii [" << wm.width << "x" << wm.height << "]";
    return os;
}
