#include "Unit.hpp"
#include <algorithm>

int Unit::unitCount = 0;

Unit::Unit(std::string n, UnitType t, int hp, int atk, int x, int y)
    : name(std::move(n)), type(t), health(hp), maxHealth(hp),
      attack(atk), posX(x), posY(y), level(1) {
    unitCount++;
}

void Unit::train() {
    level++;
    maxHealth += 20;
    health = maxHealth;
}

void Unit::moveTo(int x, int y) {
    posX = x;
    posY = y;
}

void Unit::takeDamage(int dmg) {
    health = std::max(0, health - dmg);
}

// Implementari specifice (Polimorfism in actiune)

void Infantry::applySpecialBonus() {
    health += 30; // Infanteria primeste mai mult HP
}

void Cavalry::applySpecialBonus() {
    attack += 15; // Cavaleria primeste mai mult atac
}

void Archer::applySpecialBonus() {
    // Arcasii sunt mai vulnerabili dar level-up-ul e mai eficient
    attack += 10;
    health += 5;
}