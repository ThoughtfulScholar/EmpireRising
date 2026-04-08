#ifndef WORLD_TILE_HPP
#define WORLD_TILE_HPP

#include "Common.hpp"
#include "City.hpp"
#include <memory>

class WorldTile {
private:
    TileType type;
    std::unique_ptr<City> city; // Compoziție cu Smart Pointer (Tema 2)
    bool walkable;

public:
    // Tema 1: Listă de inițializare
    explicit WorldTile(TileType t = TileType::GRASS)
        : type(t), city(nullptr), walkable(t != TileType::WATER && t != TileType::MOUNTAIN) {}

    // Rule of Three: Dezactivăm copierea pentru că avem unique_ptr,
    // dar permitem mutarea (move semantics)
    WorldTile(const WorldTile&) = delete;
    WorldTile& operator=(const WorldTile&) = delete;
    WorldTile(WorldTile&&) noexcept = default;
    WorldTile& operator=(WorldTile&&) noexcept = default;
    ~WorldTile() = default;

    void setCity(std::unique_ptr<City> c) {
        city = std::move(c);
        walkable = true; // Orașele sunt întotdeauna accesibile
    }

    [[nodiscard]] City* getCity() const { return city.get(); }
    [[nodiscard]] TileType getType() const { return type; }
    [[nodiscard]] bool isWalkable() const { return walkable; }
    [[nodiscard]] bool hasCity() const { return city != nullptr; }
};

#endif