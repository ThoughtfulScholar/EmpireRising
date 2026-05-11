#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "ArmyManager.hpp"
#include "WorldMap.hpp"
#include "Zone.hpp"
#include "Exceptions.hpp"
#include <string>
#include <vector>

class Player {
private:
    std::string name;
    int gold;
    int posX, posY;
    ArmyManager army;

public:
    Player() : name("Necunoscut"), gold(0), posX(2), posY(2) {}
    Player(std::string n, int startGold);

    // Economie
    void earnGold(int amount) { gold += amount; }
    void removeGold(int amount); // Aruncă excepție dacă nu sunt bani
    [[nodiscard]] int getGold() const { return gold; }

    // Mișcare
    void move(int dx, int dy, const WorldMap& wm);
    
    // Recrutare & Limite
    void recruit(std::unique_ptr<Unit> u, int cost, int limit);
    [[nodiscard]] int getUnitLimit(const std::vector<Zone>& regions) const;

    // Getteri
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
    [[nodiscard]] ArmyManager& getArmy() { return army; }
    [[nodiscard]] const std::string& getName() const { return name; }
};

#endif
