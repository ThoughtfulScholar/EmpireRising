// #include "Unit.h"
#include "../include/Unit.h"

// Inițializare contor static global pentru unități
int Unit::totalUnitsCount = 0;

Unit::Unit(std::string uname, int uatk, int uhp, int uupkeep)
    : name(std::move(uname)), baseAtk(uatk), maxHP(uhp), currentHP(uhp), upkeepCost(uupkeep) {
    totalUnitsCount++;
}

Unit::~Unit() {
    totalUnitsCount--;
}

int Unit::calculateTotalAttack() const {
    return getEffectiveAttack();
}

void Unit::display(std::ostream& os) const {
    print(os);
}

void Unit::takeDamage(int amount) {
    currentHP -= amount;
    if (currentHP < 0) currentHP = 0;
}

bool Unit::isDead() const {
    return currentHP <= 0;
}

std::string Unit::getName() const { return name; }
int Unit::getHP() const { return currentHP; }
int Unit::getUpkeep() const { return upkeepCost; }
int Unit::getTotalUnits() { return totalUnitsCount; }

void Unit::print(std::ostream& os) const {
    os << name << " (HP: " << currentHP << "/" << maxHP << ", Atk: " << baseAtk << ")";
}

std::ostream& operator<<(std::ostream& os, const Unit& u) {
    u.display(os);
    return os;
}

// --- IMPLEMENTĂRI INFANTERIE ---
Infantry::Infantry(std::string uname, int uatk, int uhp, int uupkeep) : Unit(std::move(uname), uatk, uhp, uupkeep) {}
std::unique_ptr<Unit> Infantry::clone() const { return std::make_unique<Infantry>(*this); }
int Infantry::getEffectiveAttack() const { return baseAtk; }
void Infantry::print(std::ostream& os) const { os << "[Infanterie] "; Unit::print(os); }

// --- IMPLEMENTĂRI ARCAȘI ---
Archer::Archer(std::string uname, int uatk, int uhp, int uupkeep) : Unit(std::move(uname), uatk, uhp, uupkeep) {}
std::unique_ptr<Unit> Archer::clone() const { return std::make_unique<Archer>(*this); }
int Archer::getEffectiveAttack() const { return static_cast<int>(baseAtk * 1.1); } // Bonus 10% distanță
void Archer::print(std::ostream& os) const { os << "[Archer] "; Unit::print(os); }

// --- IMPLEMENTĂRI CAVALERIE ---
Cavalry::Cavalry(std::string uname, int uatk, int uhp, int uupkeep) : Unit(std::move(uname), uatk, uhp, uupkeep) {}
std::unique_ptr<Unit> Cavalry::clone() const { return std::make_unique<Cavalry>(*this); }
int Cavalry::getEffectiveAttack() const { return baseAtk + 15; } // Bonus fix sarjă
void Cavalry::print(std::ostream& os) const { os << "[Cavalry] "; Unit::print(os); }

// --- IMPLEMENTĂRI GARDĂ ---
GarrisonGuard::GarrisonGuard(std::string uname, int uatk, int uhp, int uupkeep) : Unit(std::move(uname), uatk, uhp, uupkeep) {}
std::unique_ptr<Unit> GarrisonGuard::clone() const { return std::make_unique<GarrisonGuard>(*this); }
int GarrisonGuard::getEffectiveAttack() const { return baseAtk - 2; } // Defensiv, atac ușor redus
void GarrisonGuard::print(std::ostream& os) const { os << "[GarrisonGuard] "; Unit::print(os); }

// --- IMPLEMENTĂRI EROU ---
Hero::Hero(std::string uname, int uatk, int uhp, int uupkeep) : Unit(std::move(uname), uatk, uhp, uupkeep) {}
std::unique_ptr<Unit> Hero::clone() const { return std::make_unique<Hero>(*this); }
int Hero::getEffectiveAttack() const { return baseAtk * 2; } // Atac devastator dublu
void Hero::print(std::ostream& os) const { os << "[Hero!!] "; Unit::print(os); }