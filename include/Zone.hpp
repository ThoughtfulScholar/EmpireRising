#ifndef ZONE_HPP
#define ZONE_HPP

#include "City.hpp"
#include "ArmyManager.hpp"
#include "Exceptions.hpp"
#include <raylib-cpp.hpp>

class Zone {
private:
    std::string zoneName;
    City localCity;
    ArmyManager enemyGarrison; // Inamicii care păzesc orașul înainte de cucerire
    raylib::Color mapTint;

public:
    Zone(std::string name, City city, raylib::Color tint);

    // Metoda de luptă (Logică polimorfică de asediu)
    [[nodiscard]] std::string executeBattleRound(ArmyManager& playerArmy);

    // Getters
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const City& getCity() const { return localCity; }
    [[nodiscard]] ArmyManager& getEnemyGarrison() { return enemyGarrison; }
    
    friend std::ostream& operator<<(std::ostream& os, const Zone& z);
};

#endif
