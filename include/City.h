#ifndef CITY_H
#define CITY_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <utility>
#include "Unit.h" // Necesar pentru clasa Unit
#include <raylib-cpp.hpp>

class City {
private:
    std::string name;
    int posX, posY;
    int population;
    int cityLevel;
    bool occupied;

    // TEMA 2: Gestiune automată a memoriei pentru unitățile de garnizoană
    std::vector<std::unique_ptr<Unit>> garrison;

public:
    // Constructor principal
    City(std::string n, int x, int y, int pop = 150);

    // RULE OF THREE: Necesar pentru a copia corect unique_ptr prin clonare (Tema 2)
    City(const City& other);
    City& operator=(const City& other);

    // --- MECANICI DE EVOLUȚIE (TEMA 1) ---
    void upgrade();
    void growPopulation();

    // --- LOGICA MATEMATICĂ DE REBELIUNE (1 unitate la 100 oameni) ---
    // ATENȚIE: Metodele template TREBUIE să rămână integral în .h, altfel dau erori de link-utare
    template<typename T>
    void checkRebellion(T& logger) {
        if (!occupied) return;

        // Formula cerută: 1 unitate la 100 de locuitori
        int unitsNeeded = population / 100;
        if (unitsNeeded < 1) unitsNeeded = 1; // Minim o unitate pentru control

        int currentGarrisonCount = (int)garrison.size();

        // Dacă garnizoana este insuficientă, calculăm riscul
        if (currentGarrisonCount < unitsNeeded) {
            // Exemplu: Pop 800 -> needed 8. Ai 1 unitate. Șansă: (8-1)/8 = 87.5%
            float failChance = (float)(unitsNeeded - currentGarrisonCount) / unitsNeeded;
            float roll = (float)GetRandomValue(0, 100) / 100.0f;

            if (roll < failChance) {
                occupied = false;
                garrison.clear(); // Garnizoana este eliminată de revoltați (Tema 2 - memorie eliberată)
                logger.add("RASCOALA în " + name + "! Populatia a preluat controlul (Risc: " + std::to_string((int)(failChance*100)) + "%)");
            }
        }
    }

    // --- ECONOMIE (TAXE ȘI SALARII) ---
    [[nodiscard]] int collectTaxes() const;
    [[nodiscard]] int getGarrisonUpkeep() const;

    // --- GESTIONARE TRUPE ---
    void addUnitToGarrison(std::unique_ptr<Unit> u);
    std::unique_ptr<Unit> extractUnit();

    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
    [[nodiscard]] int getPopulation() const { return population; }
    [[nodiscard]] int getCityLevel() const { return cityLevel; }
    [[nodiscard]] int getUpgradeCost() const { return cityLevel * 500; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    void setOccupied(bool state) { occupied = state; }

    std::vector<std::unique_ptr<Unit>>& getGarrison() { return garrison; }
    [[nodiscard]] std::map<std::string, int> getGarrisonCounts() const;

    // Operator friend declarer
    friend std::ostream& operator<<(std::ostream& os, const City& c);
};

#endif // CITY_H