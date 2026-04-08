#include "EmpireEngine.hpp"
#include <algorithm>

EmpireEngine::EmpireEngine()
    : player("Comandant"), worldMap(16, 10), currentState(GameState::LOGIN),
      selectedUnitIndex(-1), cursorX(0), cursorY(0), currentTurn(1), inputName("") {
    worldMap.generateRandomMap();
    spawnInitialEnemies();
}

void EmpireEngine::spawnInitialEnemies() {
    for (int y = 0; y < worldMap.getHeight(); ++y) {
        for (int x = 0; x < worldMap.getWidth(); ++x) {
            WorldTile* t = worldMap.getTile(x, y);
            if (t && t->hasCity() && !t->getCity()->isPlayerOwned()) {
                // Punem un inamic "Barbar" lângă orașele roșii
                enemyArmy.push_back(std::make_unique<Infantry>("Barbar", x, (y < 9 ? y + 1 : y - 1)));
            }
        }
    }
}

void EmpireEngine::resolveCombat(Unit& attacker, int targetX, int targetY) {
    for (auto it = enemyArmy.begin(); it != enemyArmy.end(); ) {
        auto [ex, ey] = (*it)->getPosition();
        if (ex == targetX && ey == targetY) {
            int damage = attacker.getAttackValue();
            (*it)->takeDamage(damage);
            gameLogger.addLog(attacker.getName() + " a dat " + std::to_string(damage) + " dmg");

            if (!(*it)->isAlive()) {
                it = enemyArmy.erase(it);
                gameLogger.addLog("Inamic eliminat!");
                attacker.train(); // Polimorfism: Crește nivelul/HP
            } else {
                attacker.takeDamage(15); // Contraatac
                gameLogger.addLog("Inamicul a contraatacat!");
                ++it;
            }
            return;
        } else { ++it; }
    }
}

void EmpireEngine::moveSelectedUnit(int dx, int dy) {
    if (selectedUnitIndex == -1) return;

    auto& army = player.getArmy();
    auto [oldX, oldY] = army[static_cast<std::size_t>(selectedUnitIndex)]->getPosition();
    int newX = oldX + dx;
    int newY = oldY + dy;

    // Verificăm dacă atacăm un inamic
    for (const auto& eu : enemyArmy) {
        auto [ex, ey] = eu->getPosition();
        if (ex == newX && ey == newY) {
            resolveCombat(*army[static_cast<std::size_t>(selectedUnitIndex)], newX, newY);
            return;
        }
    }

    WorldTile* target = worldMap.getTile(newX, newY);
    if (target && target->isWalkable()) {
        army[static_cast<std::size_t>(selectedUnitIndex)]->moveTo(newX, newY);
        cursorX = newX;
        cursorY = newY;
    }
}

void EmpireEngine::upgradeCurrentCity() {
    WorldTile* tile = worldMap.getTile(cursorX, cursorY);
    if (tile && tile->hasCity() && tile->getCity()->isPlayerOwned()) {
        int cost = tile->getCity()->getLevel() * 200;
        if (player.spendResources(cost, 50)) {
            tile->getCity()->upgrade();
            gameLogger.addLog("Oras UPGRADAT!");
        } else gameLogger.addLog("Resurse insuficiente!");
    }
}

void EmpireEngine::recruitAtCursor(int type) {
    WorldTile* tile = worldMap.getTile(cursorX, cursorY);
    if (tile && tile->hasCity() && tile->getCity()->isPlayerOwned()) {
        if (player.spendResources(200, 0)) {
            std::string n = "Unitate_" + std::to_string(Unit::getTotalUnits());
            if (type == 1) player.recruit(std::make_unique<Infantry>(n, cursorX, cursorY));
            gameLogger.addLog("Unitate recrutata!");
        }
    }
}

void EmpireEngine::processTurn() {
    currentTurn++;
    int income = 0;
    for (int y = 0; y < worldMap.getHeight(); ++y) {
        for (int x = 0; x < worldMap.getWidth(); ++x) {
            WorldTile* t = worldMap.getTile(x, y);
            if (t && t->hasCity() && t->getCity()->isPlayerOwned())
                income += t->getCity()->getProduction();
        }
    }
    player.addResources(income, 30);
    gameLogger.addLog("Tura noua! Venit: " + std::to_string(income));
}
void EmpireEngine::drawWorld() {
    ClearBackground(GetColor(0x121212FF));
    const int tileSize = 45;
    const int offX = 50, offY = 70;

    // Grid, Relief și Orașe
    for (int y = 0; y < worldMap.getHeight(); ++y) {
        for (int x = 0; x < worldMap.getWidth(); ++x) {
            WorldTile* t = worldMap.getTile(x, y);
            Color tCol = (t->getType() == TileType::WATER) ? BLUE :
                         (t->getType() == TileType::MOUNTAIN) ? GRAY :
                         (t->getType() == TileType::FOREST) ? DARKGREEN : DARKGRAY;

            DrawRectangle(offX + x * tileSize, offY + y * tileSize, tileSize - 1, tileSize - 1, tCol);

            if (t->hasCity()) {
                Color cCol = t->getCity()->isPlayerOwned() ? GOLD : MAROON;
                DrawRectangle(offX + x * tileSize + 10, offY + y * tileSize + 10, tileSize - 20, tileSize - 20, cCol);
                DrawText(t->getCity()->getName().c_str(), offX + x * tileSize, offY + y * tileSize - 12, 10, WHITE);
            }
        }
    }

    // Inamici (Roșu) și Unități Jucător (Violet/Alb dacă e selectată)
    for (const auto& eu : enemyArmy) {
        auto [ex, ey] = eu->getPosition();
        DrawCircle(offX + ex * tileSize + tileSize/2, offY + ey * tileSize + tileSize/2, 14, RED);
    }
    const auto& army = player.getArmy();
    for (std::size_t i = 0; i < army.size(); ++i) {
        auto [ux, uy] = army[i]->getPosition();
        Color uCol = (static_cast<int>(i) == selectedUnitIndex) ? RAYWHITE : VIOLET;
        DrawCircle(offX + ux * tileSize + tileSize/2, offY + uy * tileSize + tileSize/2, 16, uCol);
    }

    DrawRectangleLines(offX + cursorX * tileSize, offY + cursorY * tileSize, tileSize, tileSize, YELLOW);
    DrawText(TextFormat("AUR: %i | TURA: %i | NUME: %s", player.getGold(), currentTurn, player.getName().c_str()), 50, 30, 20, GOLD);

    int logY = 550;
    for (const auto& m : gameLogger.getLogs()) { DrawText(m.c_str(), 50, logY, 15, LIGHTGRAY); logY += 20; }
}

void EmpireEngine::handleInput() {
    if (IsKeyPressed(KEY_ENTER) && currentState == GameState::LOGIN && !inputName.empty()) {
        player = Player(inputName);
        player.recruit(std::make_unique<Infantry>("Garda", 0, 0)); // Unitate de start
        currentState = GameState::WORLD_VIEW;
    }

    if (currentState == GameState::WORLD_VIEW) {
        if (IsKeyPressed(KEY_RIGHT)) cursorX = std::min(cursorX + 1, worldMap.getWidth() - 1);
        if (IsKeyPressed(KEY_LEFT))  cursorX = std::max(cursorX - 1, 0);
        if (IsKeyPressed(KEY_DOWN))  cursorY = std::min(cursorY + 1, worldMap.getHeight() - 1);
        if (IsKeyPressed(KEY_UP))    cursorY = std::max(cursorY - 1, 0);

        if (IsKeyPressed(KEY_S)) {
            selectedUnitIndex = -1;
            const auto& army = player.getArmy();
            for (std::size_t i = 0; i < army.size(); ++i) {
                auto [ux, uy] = army[i]->getPosition();
                if (ux == cursorX && uy == cursorY) { selectedUnitIndex = static_cast<int>(i); break; }
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) selectedUnitIndex = -1;

        if (selectedUnitIndex != -1) {
            if (IsKeyPressed(KEY_W)) moveSelectedUnit(0, -1);
            if (IsKeyPressed(KEY_A)) moveSelectedUnit(-1, 0);
            if (IsKeyPressed(KEY_D)) moveSelectedUnit(1, 0);
            if (IsKeyPressed(KEY_X)) moveSelectedUnit(0, 1);
        }

        if (IsKeyPressed(KEY_U)) upgradeCurrentCity();
        if (IsKeyPressed(KEY_R)) recruitAtCursor(1);
        if (IsKeyPressed(KEY_SPACE)) processTurn();
    }
}

void EmpireEngine::drawLogin() {
    ClearBackground(BLACK);
    DrawText("EMPIRE RISING", 350, 200, 40, GOLD);
    DrawText("Introdu nume:", 400, 300, 20, WHITE);
    int key = GetCharPressed();
    while (key > 0) { if (inputName.length() < 12) inputName += (char)key; key = GetCharPressed(); }
    if (IsKeyPressed(KEY_BACKSPACE) && !inputName.empty()) inputName.pop_back();
    DrawText(inputName.c_str(), 450, 350, 25, YELLOW);
}

void EmpireEngine::run() {
    raylib::Window window(900, 700, "Empire Strategy");
    SetTargetFPS(60);
    while (!window.ShouldClose()) {
        handleInput();
        BeginDrawing();
        if (currentState == GameState::LOGIN) drawLogin();
        else drawWorld();
        EndDrawing();
    }
}