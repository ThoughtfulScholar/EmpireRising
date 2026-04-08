#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <string>
#include "WorldTile.hpp"

class Map {
private:
    int width;
    int height;
    // Vector de vectori de pointeri pentru a gestiona mutarea obiectelor WorldTile
    std::vector<std::vector<std::unique_ptr<WorldTile>>> grid;

    static int mapInstances;

public:
    Map(int w, int h);

    // Generare Randomizată (Cerința ta)
    void generateRandomMap();

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] WorldTile* getTile(int x, int y) const;

    void addCityAt(int x, int y, const std::string& name, bool isPlayer);
};

#endif