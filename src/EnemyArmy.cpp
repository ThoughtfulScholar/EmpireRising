#include "../include/EnemyArmy.h"
#include <raylib.h>
#include <cmath>   // Pentru std::abs

// Constructorul clasei EnemyArmy
EnemyArmy::EnemyArmy(int x, int y, int difficulty) : posX(x), posY(y) {
    // Generăm trupe în funcție de dificultate
    for(int i = 0; i < difficulty; ++i) {
        troops.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Jafuitor"));
    }
    updateStats();
}

void EnemyArmy::updateStats() {
    int newHP = 0;
    int newAtk = 0;
    for (const auto& u : troops.getUnits()) {
        newHP += u->getHP(); // Adunăm viața RĂMASĂ a fiecărei unități
        newAtk += u->calculateTotalAttack();
    }
    this->totalHP = newHP;
    this->totalAtk = newAtk;
}

void EnemyArmy::moveTowards(int targetX, int targetY, const WorldMap& wm) {
    // Calculăm direcția dorită (-1, 0, sau 1)
    int dx = (targetX > posX) ? 1 : (targetX < posX ? -1 : 0);
    int dy = (targetY > posY) ? 1 : (targetY < posY ? -1 : 0);

    // Încercăm să ne mișcăm pe axa unde distanța este mai mare (pentru un aspect mai natural)
    if (std::abs(targetX - posX) > std::abs(targetY - posY)) {
        // Încercăm X primul
        if (dx != 0 && wm.isValidMove(posX + dx, posY)) {
            posX += dx;
        }
        // Dacă X e blocat de teren, încercăm Y
        else if (dy != 0 && wm.isValidMove(posX, posY + dy)) {
            posY += dy;
        }
    } else {
        // Încercăm Y primul
        if (dy != 0 && wm.isValidMove(posX, posY + dy)) {
            posY += dy;
        }
        // Dacă Y e blocat de teren, încercăm X
        else if (dx != 0 && wm.isValidMove(posX + dx, posY)) {
            posX += dx;
        }
    }
}

void EnemyArmy::takeDamage(int dmg) {
    if (troops.isEmpty()) return;

    // 1. Aplicăm damage unității din prima linie
    auto frontUnit = troops.getFrontUnit();
    frontUnit->takeDamage(dmg);

    // 2. Dacă unitatea a murit, o scoatem din armată
    troops.removeDeadUnits();

    // 3. Recalculăm statisticile totale (HP și Atac) imediat
    updateStats();
}

void EnemyArmy::draw(int offX, int offY) const {
    int px = offX + posX * 40 + 20;
    int py = offY + posY * 40 + 20;
    DrawPoly({(float)px, (float)py}, 4, 16, 0, RED);
    DrawText(TextFormat("HP:%i", totalHP), px - 15, py + 18, 10, RED);
}