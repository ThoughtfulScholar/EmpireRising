//#include "ArmyManager.h"
#include "../include/ArmyManager.h"

ArmyManager::ArmyManager(const ArmyManager& other) {
    units.reserve(other.units.size());
    for (const auto& u : other.units) {
        if (u) {
            units.push_back(u->clone());
        }
    }
}

void ArmyManager::swap(ArmyManager& other) noexcept {
    using std::swap;
    swap(this->units, other.units);
}

ArmyManager& ArmyManager::operator=(ArmyManager other) {
    this->swap(other);
    return *this;
}

void swap(ArmyManager& a, ArmyManager& b) noexcept {
    a.swap(b);
}

void ArmyManager::addUnit(std::unique_ptr<Unit> u) {
    if (u) {
        units.push_back(std::move(u));
    }
}

void ArmyManager::removeDeadUnits() {
    std::erase_if(units, [](const std::unique_ptr<Unit>& u) {
        return !u || u->isDead();
    });
}

void ArmyManager::clear() {
    units.clear();
}

bool ArmyManager::isEmpty() const {
    return units.empty();
}

size_t ArmyManager::size() const {
    return units.size();
}

std::unique_ptr<Unit> ArmyManager::extractFrontUnit() {
    if (units.empty()) return nullptr;
    auto front = std::move(units.front());
    units.erase(units.begin());
    return front;
}

Unit* ArmyManager::getFrontUnit() {
    if (units.empty()) return nullptr;
    return units.front().get();
}

const std::vector<std::unique_ptr<Unit>>& ArmyManager::getUnits() const {
    return units;
}

std::vector<std::unique_ptr<Unit>>& ArmyManager::getUnits() {
    return units;
}

int ArmyManager::calculateTotalUpkeep() const {
    int total = 0;
    for (const auto& u : units) {
        if (u) total += u->getUpkeep();
    }
    return total;
}

std::map<std::string, int> ArmyManager::getUnitCounts() const {
    std::map<std::string, int> counts;
    for (const auto& u : units) {
        if (u) {
            counts[u->getName()]++;
        }
    }
    return counts;
}