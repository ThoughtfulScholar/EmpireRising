#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <memory>
#include <raylib-cpp.hpp>

#include "Enums.h"      // Pentru GameState, TerrainType, GameData, UnitType etc.
#include "Player.h"
#include "WorldMap.h"
#include "StaticUtils.h"
#include "Zone.h"
#include "Exceptions.h"
#include "EnemyArmy.h"
#include "UnitFactory.h"
#include "LoginUI.h"

class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    Player player;
    WorldMap worldMap;
    WorldClock clock;
    GameEngine::Logger logger;

    std::vector<Zone> regions;
    std::vector<EnemyArmy> roamingEnemies;
    EnemyArmy* targetedEnemy = nullptr; // Inamicul de lângă jucător

    int activeRegionIdx = 0;
    bool showRecruitment = false;
    bool gameOver = false;
    bool showGarrisonMenu = false;
    bool showTakeMenu = false;

    static constexpr int MAX_DAYS = 50;

    // --- LOGICA ECONOMICĂ ---
    [[nodiscard]] int calculateTotalIncome() const;
    [[nodiscard]] int calculateTotalUpkeep() const;

    // --- LOGICA INTERNĂ SIMULARE ---
    void initWorld();
    void processNextDay();
    void handleInput();
    void drawUI();

public:
    Simulation();
    void run();
};

#endif // SIMULATION_H