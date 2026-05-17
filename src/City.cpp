//#include "City.h"
#include "../include/City.h"


City::City(std::string cname, int x, int y)
    : name(std::move(cname)), posX(x), posY(y), population(500), level(1), occupied(false) {}

void City::growPopulation() {
    population += 10 + level * 5;
}

int City::collectTaxes() const {
    return population / 10 * level;
}

void City::checkRebellion(GameEngine::Logger& logger) {
    int neededGarrison = population / 100;
    if (neededGarrison < 1) neededGarrison = 1;

    if ((int)garrison.size() < neededGarrison) {
        int roll = GameEngine::RandomGen::GetInt(0, 100);
        if (roll < 20) { // 20% șanse de revoltă dacă garnizoana e insuficientă
            occupied = false;
            garrison.clear();
            logger.add("!! REVOLTA: Cetatea " + name + " a izbucnit in rebeliune si a alungat garnizoana!");
        }
    }
}

void City::upgrade() {
    if (level < 5) {
        level++;
    }
}

int City::getUpgradeCost() const {
    return level * 500;
}

int City::getGarrisonUpkeep() const {
    return garrison.calculateTotalUpkeep();
}

void City::addUnitToGarrison(std::unique_ptr<Unit> u) {
    garrison.addUnit(std::move(u));
}

std::unique_ptr<Unit> City::extractUnit() {
    auto u = garrison.extractFrontUnit();
    garrison.removeDeadUnits();
    return u;
}

std::string City::getName() const { return name; }
std::pair<int, int> City::getPos() const { return {posX, posY}; }
int City::getPopulation() const { return population; }
int City::getCityLevel() const { return level; }
bool City::isOccupied() const { return occupied; }
void City::setOccupied(bool state) { occupied = state; }

const ArmyManager& City::getGarrison() const {
    return garrison;
}

ArmyManager& City::getGarrison() {
    return garrison;
}

std::map<std::string, int> City::getGarrisonCounts() const {
    return garrison.getUnitCounts();
}