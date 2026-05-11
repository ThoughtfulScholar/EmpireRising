#include "../include/EnemyArmy.hpp"
#include "../include/UnitFactory.hpp"
#include <cmath>

EnemyArmy::EnemyArmy(int x, int y, int difficulty) : posX(x), posY(y), totalHP(0), totalAtk(0) {
    for(int i = 0; i < difficulty; ++i) {
        troops.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Jafuitor"));
    }
    updateStats();
}

void EnemyArmy::updateStats() {
    int newHP = 0;
    int newAtk = 0;
    for (const auto& u : troops.getUnits()) {
        newHP += u->getHP();
        newAtk += u->calculateTotalAttack();
    }
    this->totalHP = newHP;
    this->totalAtk = newAtk;
}

void EnemyArmy::moveTowards(int targetX, int targetY, const WorldMap& wm) {
    int dx = (targetX > posX) ? 1 : (targetX < posX ? -1 : 0);
    int dy = (targetY > posY) ? 1 : (targetY < posY ? -1 : 0);

    if (std::abs(targetX - posX) > std::abs(targetY - posY)) {
        if (dx != 0 && wm.isValidMove(posX + dx, posY)) posX += dx;
        else if (dy != 0 && wm.isValidMove(posX, posY + dy)) posY += dy;
    } else {
        if (dy != 0 && wm.isValidMove(posX, posY + dy)) posY += dy;
        else if (dx != 0 && wm.isValidMove(posX + dx, posY)) posX += dx;
    }
}

void EnemyArmy::takeDamage(int dmg) {
    if (troops.isEmpty()) return;
    
    auto frontUnit = troops.getFrontUnit();
    if (frontUnit) {
        frontUnit->takeDamage(dmg);
        troops.removeDeadUnits();
        updateStats();
    }
}

void EnemyArmy::draw(int offX, int offY) const {
    int px = offX + posX * 40 + 20;
    int py = offY + posY * 40 + 20;
    DrawPoly({(float)px, (float)py}, 4, 16, 0, RED);
    DrawText(TextFormat("HP:%i", totalHP), px - 15, py + 18, 10, RED);
}
