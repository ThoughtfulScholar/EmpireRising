#ifndef ZONE_H
#define ZONE_H

#include <string>
#include <iostream>
#include <utility>
#include <raylib-cpp.hpp>
#include "City.h"
#include "ArmyManager.h"
#include "UnitFactory.h"
#include "StaticUtils.h"

class Zone {
private:
    std::string zoneName;
    City localCity;
    ArmyManager enemyGarrison; // Inamicii care păzesc orașul înainte de cucerire
    raylib::Color mapTint;

public:
    Zone(std::string name, City city, raylib::Color tint);

    [[nodiscard]] std::string executeBattleRound(ArmyManager& playerArmy);

    // Getters inline
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const City& getCity() const { return localCity; }
    [[nodiscard]] ArmyManager& getEnemyGarrison() { return enemyGarrison; }

    // Operator friend declarer
    friend std::ostream& operator<<(std::ostream& os, const Zone& z);
};

#endif // ZONE_H