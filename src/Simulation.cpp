// #include "Simulation.h"
// #include "UnitFactory.h"

#include "../include/Simulation.h"
#include "../include/UnitFactory.h"


#include <iostream>

// --- IMPLEMENTARE RAYLIB WINDOW ---
RaylibWindow::RaylibWindow(int width, int height, const std::string& title) {
    InitWindow(width, height, title.c_str());
}
RaylibWindow::~RaylibWindow() {
    CloseWindow();
}
bool RaylibWindow::ShouldClose() const {
    return WindowShouldClose();
}

// --- IMPLEMENTARE LOGIN UI ---
LoginUI::LoginUI() : letterCount(0), authenticated(false) {
    nameInput[0] = '\0';
}

void LoginUI::update() {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (letterCount < 15)) {
            nameInput[letterCount] = (char)key;
            letterCount++;
            nameInput[letterCount] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        letterCount--;
        if (letterCount < 0) letterCount = 0;
        nameInput[letterCount] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
        authenticated = true;
    }
}

void LoginUI::draw() const {
    DrawRectangle(0, 0, 1600, 1000, {20, 20, 25, 255});
    DrawText("EMPIRE RISING: RECRUITMENT & REBELLION", 450, 300, 35, GOLD);
    DrawText("INTRODUCETI NUMELE COMANDANTULUI:", 550, 420, 20, RAYWHITE);
    DrawRectangle(600, 470, 400, 50, LIGHTGRAY);
    DrawRectangleLines(600, 470, 400, 50, authenticated ? GREEN : GOLD);
    DrawText(nameInput, 610, 485, 25, BLACK);
    DrawText("APASATI [ENTER] PENTRU A INCEPE", 630, 550, 18, GRAY);
}

bool LoginUI::isAuthenticated() const { return authenticated; }
std::string LoginUI::getPlayerName() const { return nameInput; }

// --- IMPLEMENTARE SIMULATION ---
Simulation::Simulation() 
    : window(1600, 1000, "EMPIRE RISING - TEMA 2"), 
      currentState(GameState::LOGIN), 
      worldMap(30, 20), 
      selectedRecruitType(0) {
    SetTargetFPS(60);
}

void Simulation::initWorld() {
    // Generare obstacole naturale pe hartă (Munți și Ape)
    for (int x = 0; x < 30; ++x) {
        worldMap.setTile(x, 0, TerrainType::MOUNTAIN);
        worldMap.setTile(x, 19, TerrainType::MOUNTAIN);
    }
    for (int y = 5; y < 15; ++y) {
        worldMap.setTile(12, y, TerrainType::WATER);
    }

    // Configurare Provincii și Cetăți autonome
    Zone p1("Provincia de Nord");
    City c1("Castelul de Piatra", 5, 4);
    c1.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::GARDA, "Garda Veche"));
    c1.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Sulițași"));
    p1.addCity(c1);

    City c2("Fortul Dunarii", 10, 14);
    c2.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::GARDA, "Aparatori"));
    c2.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::ARCASI, "Tinstasi"));
    p1.addCity(c2);

    provinces.push_back(p1);

    Zone p2("Provincia de Est");
    City c3("Metropola Aurie", 22, 8);
    c3.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::GARDA, "Elite Regale"));
    c3.addUnitToGarrison(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Cavalerie Grea"));
    p2.addCity(c3);

    provinces.push_back(p2);

    // Plasare marcaje vizuale pentru cetăți pe matricea hărții
    for (auto& p : provinces) {
        for (auto& city : p.getCities()) {
            auto [cx, cy] = city.getPos();
            worldMap.setTile(cx, cy, TerrainType::CITY_TILE);
            worldMap.createPath(0, 0, cx, cy); // Asigură drum liber practicabil
        }
    }

    // Instanțiere hoarde de inamici mobili pe hartă
    EnemyArmy rebel1(7, 6, "Rebelii Tarani");
    rebel1.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Rascoalati"));
    activeEnemies.push_back(rebel1);

    EnemyArmy rebel2(20, 10, "Dezertori Imperiali");
    rebel2.addUnit(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Cavalerie Renegata"));
    rebel2.addUnit(UnitFactory::CreateUnit(UnitType::ARCASI, "Mercenari"));
    activeEnemies.push_back(rebel2);

    // Inițializare poziție de start jucător pe un drum liber
    player.setPos(0, 0);
    logger.add("Bine ai venit, Comandant " + player.getName() + "! Recruteaza o armata si elibereaza imperiul.");
}

void Simulation::handleInput() {
    int dx = 0, dy = 0;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))    dy = -1;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))  dy = 1;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  dx = -1;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) dx = 1;

    if (dx != 0 || dy != 0) {
        auto [px, py] = player.getPos();
        if (worldMap.isValidMove(px + dx, py + dy)) {
            player.move(dx, dy);
            processTurn(); // Fiecare mișcare consumă o zi de campanie
        }
    }

    // Schimbare tip de trupă selectat în meniul de recrutare (Tastele 1-5)
    if (IsKeyPressed(KEY_ONE))   selectedRecruitType = 0;
    if (IsKeyPressed(KEY_TWO))   selectedRecruitType = 1;
    if (IsKeyPressed(KEY_THREE)) selectedRecruitType = 2;
    if (IsKeyPressed(KEY_FOUR))  selectedRecruitType = 3;
    if (IsKeyPressed(KEY_FIVE))  selectedRecruitType = 4;

    // Executare acțiune de recrutare la apăsarea tastei R
    if (IsKeyPressed(KEY_R)) {
        UnitType t = static_cast<UnitType>(selectedRecruitType);
        try {
            player.recruitUnit(t, GameData[t].name);
            logger.add("Ai recrutat cu succes: " + GameData[t].name);
        } catch (const EmpireException& e) {
            logger.logError(e);
        }
    }
}

void Simulation::processTurn() {
    clock.nextDay();
    auto [px, py] = player.getPos();

    // 1. Verificare coliziuni și lupte cu armatele de inamici mobili
    for (auto& enemy : activeEnemies) {
        if (!enemy.isDefeated()) {
            enemy.updateAI(px, py);
            auto [ex, ey] = enemy.getPos();
            if (ex == px && ey == py) {
                logger.add("!! ATAC: Ai fost interceptat de armata mobila din factiunea: " + enemy.getFactionName());
                // Luptă directă pe loc folosind prima provincie ca arbitru tactic
                provinces[0].resolveBattle(player.getArmy(), const_cast<City&>(City("Camp deschis", px, py)), logger);
                enemy.getArmy().clear(); // Forța inamică e distrusă după luptă
            }
        }
    }

    // 2. Verificare dacă jucătorul asediază o cetate autonomă
    for (auto& p : provinces) {
        for (auto& city : p.getCities()) {
            auto [cx, cy] = city.getPos();
            if (cx == px && cy == py && !city.isOccupied()) {
                p.resolveBattle(player.getArmy(), city, logger);
            }
        }
    }

    // 3. Economie: Update zilnic cetăți controlate, colectare taxe și plăți solde
    int totalTaxCollected = 0;
    int totalGarrisonUpkeep = 0;
    bool anyCityOwned = false;
    bool allCitiesLiberated = true;

    for (auto& p : provinces) {
        for (auto& city : p.getCities()) {
            if (city.isOccupied()) {
                anyCityOwned = true;
                city.growPopulation();
                totalTaxCollected += static_cast<int>(city.collectTaxes() * WorldClock::GetGlobalTaxRate());
                totalGarrisonUpkeep += city.getGarrisonUpkeep();
                city.checkRebellion(logger);
            } else {
                allCitiesLiberated = false;
            }
        }
    }

    int playerArmyUpkeep = player.getArmy().calculateTotalUpkeep();
    int totalExpenses = playerArmyUpkeep + totalGarrisonUpkeep;

    player.addGold(totalTaxCollected);
    
    try {
        player.spendGold(totalExpenses);
    } catch (const GoldException& e) {
        logger.add("!! FALIMENT: Nu ai avut bani de solde. Armata personala a dezertat!");
        player.getArmy().clear();
    }

    // 4. Verificare condiții finale de Victorie / Înfrângere
    if (allCitiesLiberated) {
        currentState = GameState::VICTORIE;
    }
    if (clock.getDay() > 100 || (player.getArmy().isEmpty() && !anyCityOwned) || player.getGold() < 0) {
        currentState = GameState::DEFEAT;
    }
}

void Simulation::drawUI() const {
    // Stratul 1: Randare Hartă
    worldMap.draw(50, 50);

    // Stratul 2: Randare Armate de Inamici Mobili
    for (const auto& enemy : activeEnemies) {
        if (!enemy.isDefeated()) {
            auto [ex, ey] = enemy.getPos();
            DrawCircle(50 + ex * 40 + 20, 50 + ey * 40 + 20, 14, PURPLE);
            DrawText("R", 50 + ex * 40 + 15, 50 + ey * 40 + 12, 16, WHITE);
        }
    }

    // Stratul 3: Randare Cetăți (Steag Jucător verde / Inamic roșu)
    for (const auto& p : provinces) {
        for (const auto& city : p.getCities()) {
            auto [cx, cy] = city.getPos();
            DrawCircle(50 + cx * 40 + 20, 50 + cy * 40 + 20, 16, city.isOccupied() ? GOLD : BLACK);
            DrawRectangle(50 + cx * 40 + 15, 50 + cy * 40 + 15, 10, 10, city.isOccupied() ? GREEN : RED);
        }
    }

    // Stratul 4: Randare Avatar Jucător (Comandant)
    auto [px, py] = player.getPos();
    DrawCircle(50 + px * 40 + 20, 50 + py * 40 + 20, 15, BLUE);
    DrawCircleLines(50 + px * 40 + 20, 50 + py * 40 + 20, 15, WHITE);

    // Stratul 5: Panoul Lateral de Informații (UI HUD)
    DrawRectangle(1280, 0, 320, 1000, {40, 40, 45, 255});
    DrawLine(1280, 0, 1280, 1000, GOLD);

    DrawText("STATUS IMPERIU", 1300, 30, 22, GOLD);
    DrawText(TextFormat("Comandant: %s", player.getName().c_str()), 1300, 70, 16, RAYWHITE);
    DrawText(TextFormat("Ziua Campaniei: %i/100", clock.getDay()), 1300, 100, 16, RAYWHITE);
    DrawText(TextFormat("Tezaur: %i Aur", player.getGold()), 1300, 130, 16, (player.getGold() < 200) ? RED : GREEN);
    DrawText(TextFormat("Trupe: %i/%i", (int)player.getArmy().size(), player.getMaxPopulation()), 1300, 160, 16, RAYWHITE);
    DrawText(TextFormat("Upkeep Total: %i/zi", player.getArmy().calculateTotalUpkeep()), 1300, 190, 16, ORANGE);

    // Meniu interactiv de Recrutare
    DrawRectangle(1295, 230, 290, 240, {50, 50, 55, 255});
    DrawRectangleLines(1295, 230, 290, 240, GRAY);
    DrawText("CENTRUL DE RECRUTARE", 1310, 240, 16, GOLD);

    int idx = 0;
    for (const auto& [type, stats] : GameData) {
        Color c = (idx == selectedRecruitType) ? GREEN : RAYWHITE;
        DrawText(TextFormat("[%i] %s (%i G)", idx + 1, stats.name.c_str(), stats.cost), 1310, 270 + idx * 25, 14, c);
        idx++;
    }
    DrawText("APASATI [R] PENTRU RECRUTARE", 1310, 410, 13, LIGHTGRAY);
    DrawText("Sagetile / WASD pentru miscare.", 1300, 490, 14, GRAY);

    // Jurnal de Război (Ultimele 8 mesaje din Logger)
    DrawText("JURNAL DE RAZBOI", 1300, 540, 18, GOLD);
    const auto& logs = logger.getMessages();
    int startIdx = std::max(0, (int)logs.size() - 8);
    int yPos = 570;
    for (int i = startIdx; i < (int)logs.size(); ++i) {
        DrawText(logs[i].c_str(), 1295, yPos, 11, LIGHTGRAY);
        yPos += 20;
    }
}

void Simulation::run() {
    LoginUI login;
    while (!window.ShouldClose()) {
        if (currentState == GameState::LOGIN) {
            login.update();
            if (login.isAuthenticated()) {
                player = Player(login.getPlayerName(), 1500);
                initWorld();
                currentState = GameState::SIMULATION;
            }
        } else if (currentState == GameState::SIMULATION) {
            handleInput();
        }

        BeginDrawing();
        ClearBackground({30, 30, 35, 255});

        if (currentState == GameState::LOGIN) {
            login.draw();
        } else if (currentState == GameState::SIMULATION) {
            drawUI();
        } else if (currentState == GameState::VICTORIE) {
            DrawRectangle(0, 0, 1600, 1000, {0, 100, 0, 200});
            DrawText("VICTORIE SUPREMA!", 600, 450, 50, GOLD);
            DrawText(TextFormat("Ai cucerit imperiul in %i zile.", clock.getDay()), 620, 520, 20, WHITE);
        } else if (currentState == GameState::DEFEAT) {
            DrawRectangle(0, 0, 1600, 1000, {100, 0, 0, 200});
            DrawText("INFRANGERE!", 650, 450, 50, RED);
            DrawText("Timpul a expirat sau ai pierdut toate cetatile ori ai ramas fara bani, iar armata a dezertat.", 380, 520, 20, WHITE);
        }

        EndDrawing();
    }
}