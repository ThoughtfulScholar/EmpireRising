#ifndef ARMY_MANAGER_HPP
#define ARMY_MANAGER_HPP

#include "Unit.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>

class ArmyManager {
private:
    std::vector<std::unique_ptr<Unit>> units;

public:
    ArmyManager() = default;

    // --- RULE OF THREE ---
    ArmyManager(const ArmyManager& other);
    ArmyManager& operator=(ArmyManager other); // Copy and Swap idiom
    ~ArmyManager() = default;

    // Gestiune trupe
    void addUnit(std::unique_ptr<Unit> u);
    void removeDeadUnits();
    
    // Getteri și Utilitare
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] Unit* getFrontUnit();
    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getUnits() const;
    std::vector<std::unique_ptr<Unit>>& getUnits();
    
    [[nodiscard]] int calculateTotalUpkeep() const;
    [[nodiscard]] std::map<std::string, int> getUnitCounts() const;

    // Operator afișare
    friend std::ostream& operator<<(std::ostream& os, const ArmyManager& am);
};

#endif
