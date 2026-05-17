#ifndef SIMULATION_H
#define SIMULATION_H

#include "Enums.h"
#include "Terrain.h"
#include "Player.h"
#include "EnemyArmy.h"
#include "Zone.h"
#include "StaticUtils.h"
#include <vector>
#include <string>
#include <raylib.h>

// Clasă utilitară wrapper pentru inițializarea și închiderea ferestrei Raylib
class RaylibWindow {
public:
    RaylibWindow(int width, int height, const std::string& title);
    ~RaylibWindow();
    bool ShouldClose() const;
};

// Componenta de interfață pentru ecranul de autentificare/login
class LoginUI {
private:
    char nameInput[16];
    int letterCount;
    bool authenticated;

public:
    LoginUI();
    void update();
    void draw() const;
    [[nodiscard]] bool isAuthenticated() const;
    [[nodiscard]] std::string getPlayerName() const;
};

// Clasa centrală care coordonează întreaga simulare (toate sistemele integrate)
class Simulation {
private:
    RaylibWindow window;
    GameState currentState;
    WorldMap worldMap;
    Player player;
    WorldClock clock;
    GameEngine::Logger logger;
    
    std::vector<Zone> provinces;
    std::vector<EnemyArmy> activeEnemies;
    
    int selectedRecruitType; // Index pentru meniul de recrutare trupe

    void initWorld();
    void handleInput();
    void processTurn();
    void drawUI() const;

public:
    Simulation();
    void run();
};

#endif // SIMULATION_H