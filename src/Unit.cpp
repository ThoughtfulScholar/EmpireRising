#include "../include/Unit.hpp"

// Inițializarea membrului static
int Unit::totalUnitsCreated = 0;

Unit::Unit(std::string n, int h, int a, int u)
    : name(std::move(n)), hp(h), maxHp(h), atk(a),
      upkeepCost(u), level(1), xp(0) {
    totalUnitsCreated++;
}

Unit::Unit(const Unit& other)
    : name(other.name), hp(other.hp), maxHp(other.maxHp),
      atk(other.atk), upkeepCost(other.upkeepCost),
      level(other.level), xp(other.xp) {
    totalUnitsCreated++;
}

Unit& Unit::operator=(const Unit& other) {
    if (this != &other) {
        name = other.name;
        hp = other.hp;
        maxHp = other.maxHp;
        atk = other.atk;
        upkeepCost = other.upkeepCost;
        level = other.level;
        xp = other.xp;
    }
    return *this;
}

int Unit::calculateTotalAttackImpl() const {
    return atk + (level * 5);
}

int Unit::calculateTotalAttack() const {
    return calculateTotalAttackImpl();
}

void Unit::print(std::ostream& os) const {
    os << name << " [Lvl " << level << " | HP: " << hp << "/" << maxHp
       << " | ATK: " << calculateTotalAttack() << "]";
}

void Unit::display(std::ostream& os) const {
    print(os);
}

void Unit::takeDamage(int rawDamage) {
    hp -= rawDamage;
    if (hp < 0) hp = 0;
}

bool Unit::isAlive() const { 
    return hp > 0; 
}

void Unit::gainXP(int amount) {
    xp += amount;
    if (xp >= 100) {
        level++;
        xp = 0;
        maxHp += 30;
        hp = maxHp;
        atk += 10;
    }
}

std::ostream& operator<<(std::ostream& os, const Unit& u) {
    u.display(os);
    return os;
}
