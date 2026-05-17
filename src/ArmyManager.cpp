#include "../include/ArmyManager.h"
#include <algorithm>
#include <utility>

// --- RULE OF THREE: COPY CONSTRUCTOR ---
ArmyManager::ArmyManager(const ArmyManager& other) {
    for (const auto& u : other.units) {
        if (u) units.push_back(u->clone());
    }
}

// --- RULE OF THREE: COPY ASSIGNMENT (Copy and Swap) ---
ArmyManager& ArmyManager::operator=(ArmyManager other) {
    std::swap(units, other.units);
    return *this;
}

// --- METODE ---
void ArmyManager::addUnit(std::unique_ptr<Unit> u) {
    if (u) units.push_back(std::move(u));
}

void ArmyManager::removeDeadUnits() {
    std::erase_if(units, [](const auto& u) { return !u->isAlive(); });
}

bool ArmyManager::isEmpty() const {
    return units.empty();
}

Unit* ArmyManager::getFrontUnit() {
    return units.empty() ? nullptr : units.front().get();
}

const std::vector<std::unique_ptr<Unit>>& ArmyManager::getUnits() const {
    return units;
}

std::vector<std::unique_ptr<Unit>>& ArmyManager::getUnits() {
    return units;
}

int ArmyManager::calculateTotalUpkeep() const {
    int total = 0;
    for (const auto& u : units) total += u->getUpkeep();
    return total;
}

std::map<std::string, int> ArmyManager::getUnitCounts() const {
    std::map<std::string, int> counts;
    for (const auto& u : units) counts[u->getName()]++;
    return counts;
}

// --- OPERATORI ---
std::ostream& operator<<(std::ostream& os, const ArmyManager& am) {
    os << "Armata (" << am.units.size() << " unitati):\n";
    for (const auto& u : am.units) {
        os << "  - " << *u << "\n";
    }
    return os;
}


