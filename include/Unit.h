#ifndef UNIT_H
#define UNIT_H

#include "Enums.h"
#include <string>
#include <iostream>
#include <memory>

// Clasa de bază polimorfică pentru toate unitățile militare
class Unit {
protected:
    std::string name;
    int baseAtk;
    int maxHP;
    int currentHP;
    int upkeepCost;

    // Contor static cerut pentru monitorizarea obiectelor din memorie
    static int totalUnitsCount;

    // Metode virtuale protejate pentru implementarea modelului NVI
    virtual void print(std::ostream& os) const;
    [[nodiscard]] virtual int getEffectiveAttack() const = 0;

public:
    Unit(std::string uname, int uatk, int uhp, int uupkeep);
    virtual ~Unit();

    // Interfața Non-Virtuală (NVI)
    [[nodiscard]] int calculateTotalAttack() const;
    void display(std::ostream& os) const;

    // Mecanismul de clonare polimorfică (Virtual Copy Constructor)
    [[nodiscard]] virtual std::unique_ptr<Unit> clone() const = 0;

    void takeDamage(int amount);
    [[nodiscard]] bool isDead() const;

    // Getteri
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] int getHP() const;
    [[nodiscard]] int getUpkeep() const;

    static int getTotalUnits();

    // Supraîncărcarea operatorului de afișare
    friend std::ostream& operator<<(std::ostream& os, const Unit& u);
};

// --- CLASE DERIVATE SPECIFICE ---

class Infantry : public Unit {
protected:
    void print(std::ostream& os) const override;
    [[nodiscard]] int getEffectiveAttack() const override;
public:
    Infantry(std::string uname, int uatk, int uhp, int uupkeep);
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
};

class Archer : public Unit {
protected:
    void print(std::ostream& os) const override;
    [[nodiscard]] int getEffectiveAttack() const override;
public:
    Archer(std::string uname, int uatk, int uhp, int uupkeep);
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
};

class Cavalry : public Unit {
protected:
    void print(std::ostream& os) const override;
    [[nodiscard]] int getEffectiveAttack() const override;
public:
    Cavalry(std::string uname, int uatk, int uhp, int uupkeep);
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
};

class GarrisonGuard : public Unit {
protected:
    void print(std::ostream& os) const override;
    [[nodiscard]] int getEffectiveAttack() const override;
public:
    GarrisonGuard(std::string uname, int uatk, int uhp, int uupkeep);
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
};

class Hero : public Unit {
protected:
    void print(std::ostream& os) const override;
    [[nodiscard]] int getEffectiveAttack() const override;
public:
    Hero(std::string uname, int uatk, int uhp, int uupkeep);
    [[nodiscard]] std::unique_ptr<Unit> clone() const override;
};

#endif // UNIT_H