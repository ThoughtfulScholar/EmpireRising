#ifndef ENEMY_ARMY_HPP
#define ENEMY_ARMY_HPP

#include "ArmyManager.hpp"
#include "WorldMap.hpp"
#include <raylib-cpp.hpp>

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

    // Getters
    [[nodiscard]] int getX() const { return posX; }
    [[nodiscard]] int getY() const { return posY; }
    [[nodiscard]] int getHP() const { return totalHP; }
    [[nodiscard]] int getAtk() const { return totalAtk; }
    [[nodiscard]] bool isDefeated() const { return troops.isEmpty() || totalHP <= 0; }
    ArmyManager& getTroops() { return troops; }
};

#endif
