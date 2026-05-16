// #include "EnemyArmy.h"
#include "../include/EnemyArmy.h"

#include <cmath>

EnemyArmy::EnemyArmy(int x, int y, std::string faction)
    : posX(x), posY(y), factionName(std::move(faction)) {}

void EnemyArmy::updateAI(int playerX, int playerY) {
    if (isDefeated()) return;

    // Calculăm distanța Manhattan până la jucător
    int distance = std::abs(posX - playerX) + std::abs(posY - playerY);

    // Dacă jucătorul este aproape (rază de detecție de 6 căsuțe), inamicul îl vânează
    if (distance <= 6) {
        if (posX < playerX) posX++;
        else if (posX > playerX) posX--;

        if (posY < playerY) posY++;
        else if (posY > playerY) posY--;
    } else {
        // Altfel, patrulează haotic/aleator în jurul poziției sale actuale
        // (Logica de mișcare aleatoare este controlată sigur din simularea principală)
    }
}

void EnemyArmy::addUnit(std::unique_ptr<Unit> u) {
    army.addUnit(std::move(u));
}

bool EnemyArmy::isDefeated() const {
    return army.isEmpty();
}

std::pair<int, int> EnemyArmy::getPos() const { return {posX, posY}; }
std::string EnemyArmy::getFactionName() const { return factionName; }

const ArmyManager& EnemyArmy::getArmy() const { return army; }
ArmyManager& EnemyArmy::getArmy() { return army; }