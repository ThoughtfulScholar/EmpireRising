#ifndef EMPIRE_ENGINE_HPP
#define EMPIRE_ENGINE_HPP

#include "Player.hpp"
#include "Map.hpp"
#include "Logger.hpp"
#include "raylib-cpp.hpp"
#include <vector>
#include <memory>

enum class GameState { LOGIN, WORLD_VIEW };

class EmpireEngine {
private:
    Player player;
    Map worldMap;
    Logger gameLogger;
    GameState currentState;

    // Tema 2: Gestiune polimorfică (Smart Pointers)
    std::vector<std::unique_ptr<Unit>> enemyArmy;

    int selectedUnitIndex;
    int cursorX, cursorY;
    int currentTurn;
    std::string inputName;

    void handleInput();
    void processTurn();
    void resolveCombat(Unit& attacker, int targetX, int targetY);
    void moveSelectedUnit(int dx, int dy);
    void spawnInitialEnemies();

public:
    EmpireEngine();

    void drawLogin();
    void drawWorld();

    void upgradeCurrentCity();
    void recruitAtCursor(int type);

    void run();
};

#endif