#ifndef PLAYER_H
#define PLAYER_H

#include "ArmyManager.h"
#include "Exceptions.h"
#include <string>
#include <utility>

class Player {
private:
    std::string name;
    int gold;
    int maxPopulation;
    int posX;
    int posY;
    ArmyManager army;

public:
    Player();
    Player(std::string pname, int startingGold);

    void move(int dx, int dy);
    
    // Gestiune tezaur
    void addGold(int amount);
    void spendGold(int amount);
    [[nodiscard]] int getGold() const;

    // Recrutare trupe cu validarea resurselor și verificarea limitelor prin excepții
    void recruitUnit(UnitType type, const std::string& unitName);

    // Getteri și Setteri poziție
    [[nodiscard]] std::pair<int, int> getPos() const;
    void setPos(int x, int y);

    [[nodiscard]] std::string getName() const;
    [[nodiscard]] int getMaxPopulation() const;

    // Acces direct la armata mobilă
    const ArmyManager& getArmy() const;
    ArmyManager& getArmy();
};

#endif // PLAYER_H