#ifndef CITY_HPP
#define CITY_HPP

#include "Unit.hpp"
#include "GameUtils.hpp"
#include <vector>
#include <string>
#include <memory>
#include <map>

class City {
private:
    std::string name;
    int posX, posY;
    int population;
    int cityLevel;
    bool occupied;

    // Gestiune automată a memoriei pentru unitățile de garnizoană
    std::vector<std::unique_ptr<Unit>> garrison;

public:
    City(std::string n, int x, int y, int pop = 150);

    // --- RULE OF THREE (Tema 2) ---
    City(const City& other);
    City& operator=(const City& other);
    ~City() = default;

    // Evoluție și Rebeliune
    void upgrade();
    void growPopulation();
    
    template<typename T>
    void checkRebellion(T& logger);

    // Economie
    [[nodiscard]] int collectTaxes() const;
    [[nodiscard]] int getGarrisonUpkeep() const;

    // Gestiune Trupe
    void addUnitToGarrison(std::unique_ptr<Unit> u);
    std::unique_ptr<Unit> extractUnit();

    // Getteri
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
    [[nodiscard]] int getPopulation() const { return population; }
    [[nodiscard]] int getCityLevel() const { return cityLevel; }
    [[nodiscard]] int getUpgradeCost() const { return cityLevel * 500; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    void setOccupied(bool state) { occupied = state; }

    std::vector<std::unique_ptr<Unit>>& getGarrison() { return garrison; }
    [[nodiscard]] std::map<std::string, int> getGarrisonCounts() const;

    friend std::ostream& operator<<(std::ostream& os, const City& c);
};

// Implementare template în header
template<typename T>
void City::checkRebellion(T& logger) {
    if (!occupied) return;

    int unitsNeeded = population / 100;
    if (unitsNeeded < 1) unitsNeeded = 1;

    int currentGarrisonCount = (int)garrison.size();

    if (currentGarrisonCount < unitsNeeded) {
        float failChance = (float)(unitsNeeded - currentGarrisonCount) / unitsNeeded;
        float roll = (float)GetRandomValue(0, 100) / 100.0f;

        if (roll < failChance) {
            occupied = false;
            garrison.clear(); 
            logger.add("RASCOALA în " + name + "! Populatia a preluat controlul (Risc: " + std::to_string((int)(failChance*100)) + "%)");
        }
    }
}

#endif
