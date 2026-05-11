#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "GlobalEnums.hpp"
#include "WorldMap.hpp"
#include "Player.hpp"
#include "Zone.hpp"
#include "WorldClock.hpp"
#include "GameUtils.hpp"
#include "EnemyArmy.hpp"
#include "LoginUI.hpp"
#include <vector>
#include <raylib-cpp.hpp>

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
    EnemyArmy* targetedEnemy = nullptr;

    int activeRegionIdx = 0;
    bool showRecruitment = false;
    bool gameOver = false;
    bool showGarrisonMenu = false;
    bool showTakeMenu = false;

    static constexpr int MAX_DAYS = 50;

    // --- LOGICA INTERNĂ ---
    [[nodiscard]] int calculateTotalIncome() const;
    [[nodiscard]] int calculateTotalUpkeep() const;
    void initWorld();
    void processNextDay();
    void handleInput();
    
    // --- UI HELPER ---
    void drawUI();

public:
    Simulation();
    void run();
};

#endif
