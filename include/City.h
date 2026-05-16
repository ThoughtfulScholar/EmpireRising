#ifndef CITY_H
#define CITY_H

#include "ArmyManager.h"
#include "StaticUtils.h"
#include <string>
#include <utility>
#include <map>

class City {
private:
    std::string name;
    int posX;
    int posY;
    int population;
    int level;
    bool occupied; // true = aparține jucătorului, false = aparține inamicului
    ArmyManager garrison;

public:
    City(std::string cname, int x, int y);

    void growPopulation();
    [[nodiscard]] int collectTaxes() const;
    void checkRebellion(GameEngine::Logger& logger);

    void upgrade();
    [[nodiscard]] int getUpgradeCost() const;
    [[nodiscard]] int getGarrisonUpkeep() const;

    // Gestiune garnizoană
    void addUnitToGarrison(std::unique_ptr<Unit> u);
    std::unique_ptr<Unit> extractUnit();
    
    // Getteri și Setteri
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::pair<int, int> getPos() const;
    [[nodiscard]] int getPopulation() const;
    [[nodiscard]] int getCityLevel() const;
    [[nodiscard]] bool isOccupied() const;
    void setOccupied(bool state);

    // Acces direct la containerele de garnizoană
    const ArmyManager& getGarrison() const;
    ArmyManager& getGarrison();
    [[nodiscard]] std::map<std::string, int> getGarrisonCounts() const;
};

#endif // CITY_H