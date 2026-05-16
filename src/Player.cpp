// #include "Player.h"
// #include "UnitFactory.h"

#include "../include/Player.h"
#include "../include/UnitFactory.h"


Player::Player() : name("Comandant"), gold(1000), maxPopulation(20), posX(0), posY(0) {}

Player::Player(std::string pname, int startingGold)
    : name(std::move(pname)), gold(startingGold), maxPopulation(20), posX(0), posY(0) {}

void Player::move(int dx, int dy) {
    posX += dx;
    posY += dy;
}

void Player::addGold(int amount) {
    if (amount > 0) gold += amount;
}

void Player::spendGold(int amount) {
    if (gold < amount) {
        throw GoldException("Fonduri insuficiente! Ai nevoie de " + std::to_string(amount) + " aur.");
    }
    gold -= amount;
}

int Player::getGold() const { return gold; }

void Player::recruitUnit(UnitType type, const std::string& unitName) {
    const auto& stats = GameData[type];
    
    // 1. Verificare fonduri
    spendGold(stats.cost); // Aruncă GoldException dacă nu sunt bani

    // 2. Verificare limită populație
    if ((int)army.size() >= maxPopulation) {
        gold += stats.cost; // Refund în caz de eșec
        throw PopulationLimitException("Limita de populatie militara atinsa (" + std::to_string(maxPopulation) + "). Concediaza trupe sau upgradeaza cetatile!");
    }

    // 3. Creare efectivă prin fabrică
    auto newUnit = UnitFactory::CreateUnit(type, unitName);
    if (newUnit) {
        army.addUnit(std::move(newUnit));
    }
}

std::pair<int, int> Player::getPos() const { return {posX, posY}; }
void Player::setPos(int x, int y) { posX = x; posY = y; }
std::string Player::getName() const { return name; }
int Player::getMaxPopulation() const { return maxPopulation; }

const ArmyManager& Player::getArmy() const { return army; }
ArmyManager& Player::getArmy() { return army; }