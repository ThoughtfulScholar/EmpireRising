#include "Map.hpp"
#include <random>

int Map::mapInstances = 0;

Map::Map(int w, int h) : width(w), height(h) {
    mapInstances++;
    // Inițializăm grid-ul cu obiecte unice
    grid.resize(static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[static_cast<std::size_t>(y)].push_back(std::make_unique<WorldTile>(TileType::GRASS));
        }
    }
}

void Map::generateRandomMap() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> terrainDist(0, 10);

    const std::vector<std::string> enemyNames = {"Sardis", "Tarsus", "Babylon", "Susa", "Ecbatana"};
    std::size_t nameIdx = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Nu modificăm celula (0,0) - acolo va fi Capitala
            if (x == 0 && y == 0) continue;

            int val = terrainDist(gen);
            TileType t = TileType::GRASS;

            if (val == 0) t = TileType::WATER;
            else if (val == 1) t = TileType::MOUNTAIN;
            else if (val == 2) t = TileType::FOREST;

            grid[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = std::make_unique<WorldTile>(t);

            // Spawn oraș inamic random (aprox 5% șansă pe iarbă/pădure)
            if ((t == TileType::GRASS || t == TileType::FOREST) && val > 8 && nameIdx < enemyNames.size()) {
                addCityAt(x, y, enemyNames[nameIdx++], false);
            }
        }
    }

    // Capitala Jucătorului
    addCityAt(0, 0, "Capitala", true);
}

void Map::addCityAt(int x, int y, const std::string& name, bool isPlayer) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        auto& tile = grid[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
        tile->setCity(std::make_unique<City>(name, isPlayer));
    }
}

WorldTile* Map::getTile(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;
    return grid[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)].get();
}