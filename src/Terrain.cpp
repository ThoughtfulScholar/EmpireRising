// #include "Terrain.h"
#include "../include/Terrain.h"

#include <cmath>

WorldMap::WorldMap(int w, int h) : width(w), height(h) {
    grid.assign(height, std::vector<TerrainType>(width, TerrainType::PLAIN));
}

void WorldMap::setTile(int x, int y, TerrainType type) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = type;
    }
}

TerrainType WorldMap::getTile(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return grid[y][x];
    }
    return TerrainType::MOUNTAIN; // Teren sigur implicit pentru exteriorul hărții
}

int WorldMap::getWidth() const { return width; }
int WorldMap::getHeight() const { return height; }

void WorldMap::createPath(int startX, int startY, int endX, int endY) {
    int cx = startX;
    int cy = startY;

    while (cx != endX || cy != endY) {
        setTile(cx, cy, TerrainType::PLAIN); // Drumul curăță terenul impracticabil
        
        if (cx < endX) cx++;
        else if (cx > endX) cx--;
        
        if (cy < endY) cy++;
        else if (cy > endY) cy--;
    }
    setTile(endX, endY, TerrainType::PLAIN);
}

bool WorldMap::isValidMove(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    TerrainType t = grid[y][x];
    return (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
}

void WorldMap::draw(int offsetX, int offsetY) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color color = GREEN; // Implicit Câmpie (Plain)
            
            switch (grid[y][x]) {
                case TerrainType::MOUNTAIN:  color = DARKBROWN; break;
                case TerrainType::FOREST:    color = DARKGREEN; break;
                case TerrainType::WATER:     color = BLUE;      break;
                case TerrainType::CITY_TILE: color = GRAY;      break;
                default:                     color = LIME;      break;
            }

            // Desenează celula de 40x40 pixeli
            DrawRectangle(offsetX + x * 40, offsetY + y * 40, 38, 38, color);
            DrawRectangleLines(offsetX + x * 40, offsetY + y * 40, 38, 38, {0, 0, 0, 30});
        }
    }
}