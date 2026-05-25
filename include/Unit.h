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
    [[nodiscard]] virtual int calculateTotalAttackImpl() const = 0; // Transformăm în virtual pur
    virtual void print(std::ostream& os) const = 0;               // Transformăm în virtual pur

public:
    Unit(std::string n, int h, int a, int u);
    virtual ~Unit() = default;

    Unit(const Unit& other);
    Unit& operator=(const Unit& other);

    [[nodiscard]] virtual std::unique_ptr<Unit> clone() const = 0;

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
// 6. CLASE DERIVATE (Ierarhie Polimorfică - Suprascrise NVI)
// ==========================================================

class Infantry : public Unit {
protected:
    [[nodiscard]] int calculateTotalAttackImpl() const override;
    void print(std::ostream& os) const override;
public:
    // Permitem fabricii să paseze numele generat din GameData dacă se dorește
    explicit Infantry(const std::string& n = "Infanterie") : Unit(n, 300, 45, 25) {}
    [[nodiscard]] std::unique_ptr<Unit> clone() const override { return std::make_unique<Infantry>(*this); }
};

class Archer : public Unit {
protected:
    [[nodiscard]] int calculateTotalAttackImpl() const override;
    void print(std::ostream& os) const override;
public:
    explicit Archer(const std::string& n = "Arcas") : Unit(n, 180, 70, 30) {}
    [[nodiscard]] std::unique_ptr<Unit> clone() const override { return std::make_unique<Archer>(*this); }
};

class Cavalry : public Unit {
protected:
    [[nodiscard]] int calculateTotalAttackImpl() const override;
    void print(std::ostream& os) const override;
public:
    explicit Cavalry(const std::string& n = "Cavalerie") : Unit(n, 350, 60, 50) {}
    [[nodiscard]] std::unique_ptr<Unit> clone() const override { return std::make_unique<Cavalry>(*this); }
};

class GarrisonGuard : public Unit {
protected:
    [[nodiscard]] int calculateTotalAttackImpl() const override;
    void print(std::ostream& os) const override;
public:
    explicit GarrisonGuard(const std::string& n) : Unit(n, 250, 40, 15) {}
    [[nodiscard]] std::unique_ptr<Unit> clone() const override { return std::make_unique<GarrisonGuard>(*this); }
};

class Hero : public Unit {
protected:
    [[nodiscard]] int calculateTotalAttackImpl() const override;
    void print(std::ostream& os) const override;
public:
    Hero(const std::string& n, int h, int a, int u) : Unit(n, h, a, u) {}
    [[nodiscard]] std::unique_ptr<Unit> clone() const override { return std::make_unique<Hero>(*this); }
};

#endif // UNIT_H