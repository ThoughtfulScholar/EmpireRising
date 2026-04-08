#ifndef UNIT_HPP
#define UNIT_HPP

#include <string>
#include <utility>
#include "Common.hpp"

enum class UnitType { INFANTRY, CAVALRY, ARCHER };

class Unit {
protected:
    std::string name;
    UnitType type;
    int health;
    int maxHealth;
    int attack;
    int posX;
    int posY;
    int level;

    // Tema 1: Membru static pentru a contoriza unitatile create global
    static int unitCount;

public:
    Unit(std::string n, UnitType t, int hp, int atk, int x, int y);
    virtual ~Unit() = default;

    // Tema 2: Metoda virtuala pura (face clasa abstracta)
    virtual void applySpecialBonus() = 0;

    // Polimorfism: Fiecare unitate se antreneaza diferit
    virtual void train();

    // Logica de miscare pe Grid
    void moveTo(int x, int y);

    // Getteri markati cu [[nodiscard]] pentru performanta si claritate
    [[nodiscard]] std::string getName() const { return name; }
    [[nodiscard]] int getHealth() const { return health; }
    [[nodiscard]] int getAttackValue() const { return attack + (level * 5); }
    [[nodiscard]] std::pair<int, int> getPosition() const { return {posX, posY}; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] bool isAlive() const { return health > 0; }

    void takeDamage(int dmg);
    [[nodiscard]] static int getTotalUnits() { return unitCount; }
};

// --- Subclasele Polimorfice ---

class Infantry : public Unit {
public:
    explicit Infantry(std::string n, int x = 0, int y = 0)
        : Unit(std::move(n), UnitType::INFANTRY, 150, 25, x, y) {}

    void applySpecialBonus() override; // Bonus de rezistenta
};

class Cavalry : public Unit {
public:
    explicit Cavalry(std::string n, int x = 0, int y = 0)
        : Unit(std::move(n), UnitType::CAVALRY, 180, 40, x, y) {}

    void applySpecialBonus() override; // Bonus de impact (damage)
};

class Archer : public Unit {
public:
    explicit Archer(std::string n, int x = 0, int y = 0)
        : Unit(std::move(n), UnitType::ARCHER, 100, 30, x, y) {}

    void applySpecialBonus() override; // Bonus de raza/precizie
};

#endif