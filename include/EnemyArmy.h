#ifndef ENEMYARMY_H
#define ENEMYARMY_H

#include "ArmyManager.h"
#include "WorldMap.h"
#include "UnitFactory.h"

class EnemyArmy {
private:
    int posX, posY;
    ArmyManager troops;
    int totalHP;
    int totalAtk;

public:
    EnemyArmy(int x, int y, int difficulty);

    void updateStats();
    void moveTowards(int targetX, int targetY, const WorldMap& wm);
    void takeDamage(int dmg);
    void draw(int offX, int offY) const;

    // Getteri inline
    int getX() const { return posX; }
    int getY() const { return posY; }
    int getHP() const { return totalHP; }
    int getAtk() const { return totalAtk; }
    bool isDefeated() const { return troops.isEmpty() || totalHP <= 0; }
    ArmyManager& getTroops() { return troops; }
};

#endif // ENEMYARMY_H