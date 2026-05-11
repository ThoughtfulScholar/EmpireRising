#ifndef UNIT_HPP
#define UNIT_HPP

#include <iostream>
#include <string>
#include <memory>

class Unit {
protected:
    std::string name;
    int hp;
    int maxHp;
    int atk;
    int upkeepCost;
    int level;
    int xp;

    static int totalUnitsCreated; // Atribut static (Cerință Tema 2)

    // --- Metodă Virtuală Protejată (NVI) ---
    [[nodiscard]] virtual int calculateTotalAttackImpl() const;

    // --- Afișare Virtuală ---
    virtual void print(std::ostream& os) const;

public:
    Unit(std::string n, int h, int a, int u);
    virtual ~Unit() = default;

    // Rule of Three (Necesar pentru clonare)
    Unit(const Unit& other);
    Unit& operator=(const Unit& other);

    // Metoda de clonare (Cerință Tema 2)
    virtual std::unique_ptr<Unit> clone() const = 0;

    // --- Interfața Non-Virtuală (Publică) ---
    [[nodiscard]] int calculateTotalAttack() const;
    void display(std::ostream& os) const;

    // Logica de status
    virtual void takeDamage(int rawDamage);
    [[nodiscard]] bool isAlive() const;
    void gainXP(int amount);

    // Getteri
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getAtk() const { return atk; }
    [[nodiscard]] int getUpkeep() const { return upkeepCost; }

    static int getTotalUnits() { return totalUnitsCreated; }

    // Operatorul de afișare polimorfică
    friend std::ostream& operator<<(std::ostream& os, const Unit& u);
};

#endif
