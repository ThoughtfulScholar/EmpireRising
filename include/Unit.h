#ifndef UNIT_H
#define UNIT_H

#include <iostream>
#include <string>
#include <memory>
#include <utility>

class Unit {
protected:
    std::string name;
    int hp;
    int maxHp;
    int atk;
    int upkeepCost;
    int level;
    int xp;

    static int totalUnitsCreated;

    // --- 1. IMPLEMENTĂRI VIRTUALE PROTEJATE (Cerință Tema 2) ---
    [[nodiscard]] virtual int calculateTotalAttackImpl() const;
    virtual void print(std::ostream& os) const;

public:
    Unit(std::string n, int h, int a, int u);
    virtual ~Unit() = default;

    // Rule of Three: Copy Constructor
    Unit(const Unit& other);

    // Rule of Three: Copy Assignment
    Unit& operator=(const Unit& other);

    virtual std::unique_ptr<Unit> clone() const = 0;

    // --- 2. INTERFAȚA NON-VIRTUALĂ (NVI) ---
    [[nodiscard]] int calculateTotalAttack() const;
    void display(std::ostream& os) const;

    // --- LOGICA DE LUPTA ---
    virtual void takeDamage(int rawDamage);
    [[nodiscard]] bool isAlive() const;
    static void playAttackSound() {}

    void gainXP(int amount);

    // --- GETTERI ---
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] int getHP() const;
    [[nodiscard]] int getAtk() const;
    [[nodiscard]] int getUpkeep() const;

    static int getTotalUnits();

    // --- 3. OPERATORI ---
    friend std::ostream& operator<<(std::ostream& os, const Unit& u);
};

// ==========================================================
// 6. CLASE DERIVATE (Ierarhie Polimorfică - Tema 2)
// ==========================================================

class Infantry : public Unit {
public:
    Infantry() : Unit("Infanterie", 300, 45, 25) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Infantry>(*this); }
};

class Archer : public Unit {
public:
    Archer() : Unit("Arcas", 180, 70, 30) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Archer>(*this); }
};

class Cavalry : public Unit {
public:
    Cavalry() : Unit("Cavalerie", 350, 60, 50) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Cavalry>(*this); }
};

class GarrisonGuard : public Unit {
public:
    explicit GarrisonGuard(const std::string& n) : Unit(n, 250, 40, 15) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<GarrisonGuard>(*this); }
};

class Hero : public Unit {
public:
    Hero(const std::string& n, int h, int a, int u) : Unit(n, h, a, u) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Hero>(*this); }
};

#endif // UNIT_H