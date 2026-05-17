#ifndef ENEMYARMY_H
#define ENEMYARMY_H

#include "ArmyManager.h"
#include <utility>
#include <string>

class EnemyArmy {
private:
    int posX;
    int posY;
    std::string factionName;
    ArmyManager army;

public:
    EnemyArmy(int x, int y, std::string faction);

    // AI de mișcare autonomă bazat pe distanța geometrică până la jucător
    void updateAI(int playerX, int playerY);

    // Gestiune trupe inamice
    void addUnit(std::unique_ptr<Unit> u);
    [[nodiscard]] bool isDefeated() const;

    // Getteri
    [[nodiscard]] std::pair<int, int> getPos() const;
    [[nodiscard]] std::string& getFactionName() const;
    
    // Acces la armata proprie
    const ArmyManager& getArmy() const;
    ArmyManager& getArmy();
};

#endif // ENEMYARMY_H