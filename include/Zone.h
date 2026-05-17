#ifndef ZONE_H
#define ZONE_H

#include "City.h"
#include "ArmyManager.h"
#include "StaticUtils.h"
#include <string>
#include <vector>

class Zone {
private:
    std::string zoneName;
    std::vector<City> cities;

public:
    explicit Zone(std::string name);

    void addCity(const City& c);
    
    // Motorul de luptă turn-based între armata jucătorului și garnizoana unei cetăți
    bool resolveBattle(ArmyManager& playerArmy, City& targetCity, GameEngine::Logger& logger);

    // Getteri
    // [[nodiscard]] std::string getName() const;
    [[nodiscard]] const std::string& getName() const;
    const std::vector<City>& getCities() const;
    std::vector<City>& getCities();
};

#endif // ZONE_H