#ifndef ARMYMANAGER_H
#define ARMYMANAGER_H

#include "Unit.h"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>

// Manager dedicat gestiunii colectiilor de unitati (Copy-and-Swap idiom)
class ArmyManager {
private:
    std::vector<std::unique_ptr<Unit>> units;

public:
    ArmyManager() = default;

    // Rule of 3/5: Constructor de copiere bazat pe clonare polimorfica
    ArmyManager(const ArmyManager& other);

    // Operator de atribuire prin copiere (Copy-and-Swap)
    ArmyManager& operator=(ArmyManager other);

    // Move Constructor si Move Assignment implicite (unique_ptr stie nativ move)
    ArmyManager(ArmyManager&& other) noexcept = default;
    ArmyManager& operator=(ArmyManager&& other) noexcept = default;

    ~ArmyManager() = default;

    // Metoda auxiliara pentru Swap
    void swap(ArmyManager& other) noexcept;

    // Gestiune trupe
    void addUnit(std::unique_ptr<Unit> u);
    void removeDeadUnits();
    void clear();

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] size_t size() const;
    
    // Extragere si acces la prima unitate (frontline)
    std::unique_ptr<Unit> extractFrontUnit();
    Unit* getFrontUnit();

    // Metode de acces si statistici
    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getUnits() const;
    std::vector<std::unique_ptr<Unit>>& getUnits();
    
    [[nodiscard]] int calculateTotalUpkeep() const;
    [[nodiscard]] std::map<std::string, int> getUnitCounts() const;
};

// Declaratie externa pentru functia de swap de tip ADL
void swap(ArmyManager& a, ArmyManager& b) noexcept;

#endif // ARMYMANAGER_H