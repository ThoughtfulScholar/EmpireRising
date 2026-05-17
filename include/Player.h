#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <memory>
#include "ArmyManager.h"
#include "Zone.h"
#include "WorldMap.h"
#include "Enums.h" // Pentru ierarhia de excepții (PopulationLimitException etc.)
#include "Exceptions.h"

class Player {
private:
    std::string commanderName;
    int gold;
    ArmyManager army;
    int posX, posY;

public:
    explicit Player(std::string name = "Comandant", int startingGold = 1200);

    // --- LOGICA LIMITĂ POPULAȚIE ---
    [[nodiscard]] static int getUnitLimit(const std::vector<Zone>& regions);

    void recruit(std::unique_ptr<Unit> u, int cost, int maxLimit);
    void move(int dx, int dy, const WorldMap& worldMap);

    void earnGold(int amount) { gold += amount; }
    void removeGold(int amount) { gold -= amount; }

    // Getters inline
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] ArmyManager& getArmy() { return army; }
    [[nodiscard]] const ArmyManager& getArmy() const { return army; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }

    // Operator friend declarer
    friend std::ostream& operator<<(std::ostream& os, const Player& p);
};

#endif // PLAYER_H