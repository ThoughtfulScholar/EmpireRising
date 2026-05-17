#include "../include/Simulation.h"

// Constructor
Simulation::Simulation()
    : window(1800, 1000, "EMPIRE RISING - TEMA 2"),
      currentState(GameState::LOGIN),
      worldMap(30, 20) {
    SetTargetFPS(60);
}

// Logica economică
int Simulation::calculateTotalIncome() const {
    int total = 0;
    for (const auto& r : regions) {
        total += r.getCity().collectTaxes();
    }
    return static_cast<int>(total * WorldClock::GetGlobalTaxRate());
}

int Simulation::calculateTotalUpkeep() const {
    int upkeep = player.getArmy().calculateTotalUpkeep();
    for (const auto& r : regions) {
        upkeep += r.getCity().getGarrisonUpkeep();
    }
    return upkeep;
}

// Inițializare lume
void Simulation::initWorld() {
    for (int i = 0; i < 40; ++i) {
        int tx, ty;
        do {
            tx = GameEngine::RandomGen::GetInt(0, worldMap.getWidth() - 1);
            ty = GameEngine::RandomGen::GetInt(0, worldMap.getHeight() - 1);
        } while (worldMap.getTile(tx, ty) != TerrainType::PLAIN); // Verifică să fie câmpie goală

        int roll = GameEngine::RandomGen::GetInt(0, 100);
        if (roll < 15) worldMap.setTile(tx, ty, TerrainType::MOUNTAIN);
        else if (roll < 30) worldMap.setTile(tx, ty, TerrainType::FOREST);
        else if (roll < 40) worldMap.setTile(tx, ty, TerrainType::WATER);
    }
    for (int i = 0; i < 6; ++i) {
        int cx, cy;
        do {
            cx = GameEngine::RandomGen::GetInt(2, 27);
            cy = GameEngine::RandomGen::GetInt(2, 17);
        } while (worldMap.getTile(cx, cy) != TerrainType::PLAIN); // Se spawnează DOAR pe câmpie liberă

        worldMap.setTile(cx, cy, TerrainType::CITY_TILE);
        worldMap.createPath(2, 2, cx, cy);

        City c((i == 0 ? "Capitala" : "Cetate Inamica " + std::to_string(i)), cx, cy);
        if (i == 0) {
            c.setOccupied(true);
            regions.emplace_back("Provincia Natala", std::move(c), GREEN);
        } else {
            regions.emplace_back("Tinut Inamic", std::move(c), RED);
        }
    }

    /*
    for (int i = 0; i < 40; ++i) {
        int tx = GameEngine::RandomGen::GetInt(0, worldMap.getWidth() - 1);
        int ty = GameEngine::RandomGen::GetInt(0, worldMap.getHeight() - 1);

        int roll = GameEngine::RandomGen::GetInt(0, 100);
        if (roll < 15) worldMap.setTile(tx, ty, TerrainType::MOUNTAIN);
        else if (roll < 30) worldMap.setTile(tx, ty, TerrainType::FOREST);
        else if (roll < 40) worldMap.setTile(tx, ty, TerrainType::WATER);
    }

    for (int i = 0; i < 6; ++i) {
        int cx = GameEngine::RandomGen::GetInt(2, 27);
        int cy = GameEngine::RandomGen::GetInt(2, 17);

        worldMap.setTile(cx, cy, TerrainType::CITY_TILE);
        worldMap.createPath(2, 2, cx, cy);

        City c((i == 0 ? "Capitala" : "Cetate Inamica " + std::to_string(i)), cx, cy);
        if (i == 0) {
            c.setOccupied(true);
            regions.emplace_back("Provincia Natala", std::move(c), GREEN);
        } else {
            regions.emplace_back("Tinut Inamic", std::move(c), RED);
        }
    }
    */


}

// Procesare zi nouă
void Simulation::processNextDay() {
    if (gameOver) return;

    targetedEnemy = nullptr;

    clock.nextDay();
    logger.add("--- ZIUA " + std::to_string(clock.getDay()) + " ---");

    int totalIncome = 0;
    int totalUpkeep = 0;
    int occupiedCitiesCount = 0;

    totalUpkeep += player.getArmy().calculateTotalUpkeep();

    for (auto& reg : regions) {
        City& city = reg.getCity();
        if (city.isOccupied()) {
            occupiedCitiesCount++;
            city.growPopulation();
            totalIncome += (int)(city.collectTaxes() * WorldClock::GetGlobalTaxRate());
            totalUpkeep += city.getGarrisonUpkeep();
            city.checkRebellion(logger);
        }
    }

    int netProfit = totalIncome - totalUpkeep;
    player.earnGold(netProfit);
    logger.add(TextFormat("Economie: Net %+i Aur", netProfit));

    for (auto& enemy : roamingEnemies) {
        Zone* targetZone = nullptr;
        float minDistance = 999.0f;

        for (auto& reg : regions) {
            if (reg.getCity().isOccupied()) {
                auto [cx, cy] = reg.getCity().getPos();
                float dist = (float)std::sqrt(std::pow(cx - enemy.getX(), 2) + std::pow(cy - enemy.getY(), 2));
                if (dist < minDistance) {
                    minDistance = dist;
                    targetZone = &reg;
                }
            }
        }

        if (targetZone) {
            auto [tx, ty] = targetZone->getCity().getPos();

            if (minDistance < 1.5f) {
                City& targetCity = targetZone->getCity();
                logger.add("! ASEDIU: Inamicii asalteaza " + targetCity.getName() + "!");

                if (targetCity.getGarrison().empty()) {
                    targetCity.setOccupied(false);
                    logger.add("!! " + targetCity.getName() + " a cazut (Fara aparare)!");
                } else {
                    targetCity.extractUnit();
                    enemy.takeDamage(100);
                    logger.add("Garnizoana din " + targetCity.getName() + " a rezistat, dar a pierdut oameni.");
                }
            }
            else {
                // --- CAZUL NOU ADĂUGAT: JUCĂTORUL ESTE ÎN CALEA TRUPELOR INAMICE ---
                auto [pX, pY] = player.getPos();

                // Calculăm distanța matematică dintre inamic și jucător
                float distToPlayer = (float)std::sqrt(std::pow(enemy.getX() - pX, 2) + std::pow(enemy.getY() - pY, 2));

                // Dacă distanța este mai mică sau egală cu 1.5 (adică ești pe căsuța vecină sau chiar pe aceeași căsuță)
                if (distToPlayer <= 1.5f) {
                    logger.add("! INTERCEPTARE: O armata inamica s-a ciocnit de tine pe drum!");

                    if (!player.getArmy().isEmpty() && player.getArmy().getFrontUnit() != nullptr) {

                        // 1. TRUPELE TALE ATACĂ INAMICUL MAI ÎNTÂI (Calculăm damage-ul total)
                        int playerTotalAtk = 0;
                        for (const auto& u : player.getArmy().getUnits()) {
                            playerTotalAtk += u->calculateTotalAttack();
                        }

                        enemy.takeDamage(playerTotalAtk);
                        logger.add(TextFormat("Trupele tale au ripostat si au dat %i damage inamicului!", playerTotalAtk));

                        // 2. DACĂ INAMICUL N-A MURIT DIN ATACUL TĂU, ATACĂ ȘI EL
                        if (!enemy.isDefeated()) {
                            player.getArmy().getFrontUnit()->takeDamage(enemy.getAtk());
                            player.getArmy().removeDeadUnits();
                            logger.add("Trupele tale din prima linie au fost atacate in mars!");

                            if (player.getArmy().isEmpty()) {
                                logger.add("!! DEZASTRU: Toata armata ta mobila a fost nimicita in interceptare!");
                            }
                        } else {
                            // Dacă l-ai omorât în faza de interceptare, primești aur ca la tasta F
                            player.earnGold(250);
                            logger.add("VICTORIE IN MARS! Ai distrus armata inamica inainte sa poata神器 riposta.");
                        }

                    } else {
                        logger.add("! Alerta: Nu ai trupe mobile! Inamicul te-a obligat sa platesti rascumpare.");
                        player.removeGold(150);
                    }
                } else {
                    // Dacă drumul e liber și jucătorul NU e în cale, își continuă marșul normal spre cetate
                    enemy.moveTowards(tx, ty, worldMap);
                }
            }
        }
    }

    if (clock.getDay() % 6 == 0) {
        int attempts = 0;
        while (attempts < 10) {
            int sx = (GameEngine::RandomGen::GetInt(0, 1) == 0) ? 0 : 29;
            int sy = GameEngine::RandomGen::GetInt(0, 19);

            if (worldMap.isValidMove(sx, sy)) {
                roamingEnemies.emplace_back(sx, sy, 2 + (clock.getDay() / 10));
                logger.add("ALERTA: Intruziune inamica detectata!");
                break;
            }
            attempts++;
        }
    }

    std::erase_if(roamingEnemies, [](const EnemyArmy& e) { return e.isDefeated(); });

    bool allCaptured = (occupiedCitiesCount == (int)regions.size());

    if (allCaptured) {
        currentState = GameState::VICTORIE;
        gameOver = true;
        logger.add("VICTORIE: Ai unit toate tinuturile sub un singur steag!");
    }
    else if (occupiedCitiesCount <= 0 && clock.getDay() > 1) {
        currentState = GameState::DEFEAT;
        gameOver = true;
        logger.add("INFRANGERE: Ultimul bastion a cazut.");
    }
    else if (player.getGold() < -1000) {
        currentState = GameState::DEFEAT;
        gameOver = true;
        logger.add("INFRANGERE: Tezaurul este gol, armata a dezertat.");
    }
    else if (clock.getDay() >= MAX_DAYS) {
        currentState = GameState::DEFEAT;
        gameOver = true;
        logger.add("INFRANGERE: Timpul a expirat. Campania a esuat.");
    }
};

// Gestionare Input
void Simulation::handleInput() {
    if (showRecruitment || showGarrisonMenu || showTakeMenu) {
        if (IsKeyPressed(KEY_R) && showRecruitment) { showRecruitment = false; return; }
        if (IsKeyPressed(KEY_G) && showGarrisonMenu) { showGarrisonMenu = false; return; }
        if (IsKeyPressed(KEY_T) && showTakeMenu) { showTakeMenu = false; return; }

        if (showGarrisonMenu) {
            UnitType toMove;
            bool keyHit = false;
            if (IsKeyPressed(KEY_ONE))        { toMove = UnitType::INFANTERIE; keyHit = true; }
            else if (IsKeyPressed(KEY_TWO))   { toMove = UnitType::ARCASI;     keyHit = true; }
            else if (IsKeyPressed(KEY_THREE)) { toMove = UnitType::CAVALERIE;  keyHit = true; }
            else if (IsKeyPressed(KEY_FOUR))  { toMove = UnitType::GARDA;      keyHit = true; }
            else if (IsKeyPressed(KEY_FIVE))  { toMove = UnitType::EROU;       keyHit = true; }

            if (keyHit) {
                auto& pUnits = player.getArmy().getUnits();
                std::string targetName = GameData[toMove].name;

                auto it = std::find_if(pUnits.begin(), pUnits.end(), [&](const std::unique_ptr<Unit>& u) {
                    if (toMove == UnitType::INFANTERIE) return (dynamic_cast<Infantry*>(u.get()) != nullptr || u->getName() == targetName);
                    if (toMove == UnitType::ARCASI)     return (dynamic_cast<Archer*>(u.get()) != nullptr || u->getName() == targetName);
                    if (toMove == UnitType::CAVALERIE)  return (dynamic_cast<Cavalry*>(u.get()) != nullptr || u->getName() == targetName);
                    if (toMove == UnitType::GARDA)      return (dynamic_cast<GarrisonGuard*>(u.get()) != nullptr || u->getName() == targetName);
                    if (toMove == UnitType::EROU)       return (dynamic_cast<Hero*>(u.get()) != nullptr || u->getName() == targetName);
                    return false;
                });

                if (it != pUnits.end()) {
                    regions[activeRegionIdx].getCity().addUnitToGarrison(std::move(*it));
                    pUnits.erase(it);
                    logger.add("Transferat in oras: " + targetName);
                } else {
                    logger.add("! Nu ai " + targetName + " in armata.");
                }
            }
        }

        if (showTakeMenu) {
            UnitType toTake;
            bool keyHit = false;
            if (IsKeyPressed(KEY_ONE))        { toTake = UnitType::INFANTERIE; keyHit = true; }
            else if (IsKeyPressed(KEY_TWO))   { toTake = UnitType::ARCASI;     keyHit = true; }
            else if (IsKeyPressed(KEY_THREE)) { toTake = UnitType::CAVALERIE;  keyHit = true; }
            else if (IsKeyPressed(KEY_FOUR))  { toTake = UnitType::GARDA;      keyHit = true; }
            else if (IsKeyPressed(KEY_FIVE))  { toTake = UnitType::EROU;       keyHit = true; }

            if (keyHit) {
                try {
                    int limit = player.getUnitLimit(regions);
                    if ((int)player.getArmy().getUnits().size() >= limit)
                        throw PopulationLimitException("Armata mobila este plina!");

                    auto& gUnits = regions[activeRegionIdx].getCity().getGarrison();
                    std::string targetName = GameData[toTake].name;

                    auto it = std::find_if(gUnits.begin(), gUnits.end(), [&](const std::unique_ptr<Unit>& u) {
                        if (toTake == UnitType::INFANTERIE) return (dynamic_cast<Infantry*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toTake == UnitType::ARCASI)     return (dynamic_cast<Archer*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toTake == UnitType::CAVALERIE)  return (dynamic_cast<Cavalry*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toTake == UnitType::GARDA)      return (dynamic_cast<GarrisonGuard*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toTake == UnitType::EROU)       return (dynamic_cast<Hero*>(u.get()) != nullptr || u->getName() == targetName);
                        return false;
                    });

                    if (it != gUnits.end()) {
                        player.getArmy().addUnit(std::move(*it));
                        gUnits.erase(it);
                        logger.add("Recuperat din oras: " + targetName);
                    } else {
                        logger.add("! In oras nu exista " + targetName);
                    }
                } catch (const EmpireException& e) { logger.logError(e); }
            }
        }

        if (showRecruitment) {
            int typeIdx = -1;
            if (IsKeyPressed(KEY_ONE)) typeIdx = (int)UnitType::INFANTERIE;
            else if (IsKeyPressed(KEY_TWO)) typeIdx = (int)UnitType::ARCASI;
            else if (IsKeyPressed(KEY_THREE)) typeIdx = (int)UnitType::CAVALERIE;
            else if (IsKeyPressed(KEY_FOUR)) typeIdx = (int)UnitType::GARDA;
            else if (IsKeyPressed(KEY_FIVE)) typeIdx = (int)UnitType::EROU;
            if (typeIdx != -1) {
                UnitType t = static_cast<UnitType>(typeIdx);
                try {
                    player.recruit(UnitFactory::CreateUnit(t, GameData[t].name), GameData[t].cost, player.getUnitLimit(regions));
                    logger.add("Recrutat: " + GameData[t].name);
                } catch (const EmpireException& e) { logger.logError(e); }
            }
        }
        return;
    }

    auto [px, py] = player.getPos();
    targetedEnemy = nullptr;
    for (auto& enemy : roamingEnemies) {
        if (!enemy.isDefeated() && std::abs(enemy.getX() - px) <= 1 && std::abs(enemy.getY() - py) <= 1) {
            targetedEnemy = &enemy;
            break;
        }
    }

    int dx = 0, dy = 0;
    if (IsKeyPressed(KEY_W)) dy = -1;
    else if (IsKeyPressed(KEY_S)) dy = 1;
    else if (IsKeyPressed(KEY_A)) dx = -1;
    else if (IsKeyPressed(KEY_D)) dx = 1;

    if (dx != 0 || dy != 0) {
        try {
            bool blockedByEnemy = std::any_of(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
                return e.getX() == (px + dx) && e.getY() == (py + dy) && !e.isDefeated();
            });

            if (blockedByEnemy) {
                logger.add("! DRUM BLOCAT: Inamicul iti bareaza calea.");
            } else {
                player.move(dx, dy, worldMap);
            }
        } catch (const EmpireException& e) {
            logger.logError(e);
        }
    }

    if (IsKeyPressed(KEY_TAB)) activeRegionIdx = (activeRegionIdx + 1) % regions.size();
    if (IsKeyPressed(KEY_N)) processNextDay();
    if (IsKeyPressed(KEY_R)) { showRecruitment = true; return; }

    if (IsKeyPressed(KEY_F)) {
        if (targetedEnemy && !targetedEnemy->isDefeated()) {
            try {
                if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe!");
                int pAtk = 0;
                for(const auto& u : player.getArmy().getUnits()) pAtk += u->calculateTotalAttack();
                targetedEnemy->takeDamage(pAtk);
                if (!targetedEnemy->isDefeated()) {
                    player.getArmy().getFrontUnit()->takeDamage(targetedEnemy->getAtk());
                    player.getArmy().removeDeadUnits();
                } else {
                    player.earnGold(250);
                    logger.add("VICTORIE! Armata distrusa.");
                    targetedEnemy = nullptr;
                }
            } catch (const EmpireException& e) { logger.logError(e); }
        } else {
            auto& sel = regions[activeRegionIdx];
            if (px == sel.getCity().getPos().first && py == sel.getCity().getPos().second) {
                try {
                    if (!sel.getCity().isOccupied()) {
                        if (sel.getEnemyGarrison().getUnits().empty()) {
                            try {
                                sel.getEnemyGarrison().addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Infanterie Inamica"));
                                sel.getEnemyGarrison().addUnit(UnitFactory::CreateUnit(UnitType::ARCASI, "Arcasi Inamici"));
                                logger.add("Garnizoana inamica s-a regrupat pentru aparare!");
                            } catch (...) {
                                logger.add("Eroare la generarea apararii inamice.");
                            }
                        }
                    }

                    if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe!");
                    logger.add(sel.executeBattleRound(player.getArmy()));
                } catch (const EmpireException& e) {
                    logger.logError(e);
                }
            }
        }
    }

    bool foundCityUnderPlayer = false;
    for (int i = 0; i < (int)regions.size(); i++) {
        auto [cx, cy] = regions[i].getCity().getPos();
        if (px == cx && py == cy) {
            activeRegionIdx = i;
            foundCityUnderPlayer = true;
            break;
        }
    }

    auto& currentReg = regions[activeRegionIdx];

    if (foundCityUnderPlayer && currentReg.getCity().isOccupied()) {
        if (IsKeyPressed(KEY_G)) showGarrisonMenu = true;
        if (IsKeyPressed(KEY_T)) showTakeMenu = true;

        if (IsKeyPressed(KEY_U)) {
            City& c = currentReg.getCity();
            if (c.getCityLevel() < 5) {
                if (player.getGold() >= c.getUpgradeCost()) {
                    player.removeGold(c.getUpgradeCost());
                    c.upgrade();
                    logger.add("Modernizat: " + c.getName());
                } else {
                    logger.add("Eroare: Aur insuficient!");
                }
            } else {
                logger.add("Eroare: Nivel maxim atins!");
            }
        }
    }
}

// Desenare UI
void Simulation::drawUI() {
    const int sbX = 1400;
    const int mapOffX = 50, mapOffY = 50;

    worldMap.draw(mapOffX, mapOffY);

    for (size_t i = 0; i < regions.size(); ++i) {
        const auto& city = regions[i].getCity();
        auto [cx, cy] = city.getPos();
        int sx = mapOffX + cx * 40 + 20;
        int sy = mapOffY + cy * 40 + 20;

        if (i == (size_t)activeRegionIdx) {
            DrawCircleLines(sx, sy, 26, GOLD);
            DrawCircleLines(sx, sy, 27, YELLOW);
        }

        Color cCol = city.isOccupied() ? GREEN : MAROON;
        DrawCircle(sx, sy, 15, cCol);
        DrawCircleLines(sx, sy, 16, RAYWHITE);
        DrawText(city.getName().c_str(), sx - MeasureText(city.getName().c_str(), 12) / 2, sy + 22, 12, RAYWHITE);
    }

    for (const auto& enemy : roamingEnemies) {
        int ex = mapOffX + enemy.getX() * 40 + 20;
        int ey = mapOffY + enemy.getY() * 40 + 20;
        DrawPoly({(float)ex, (float)ey}, 4, 14, 0, RED);
        DrawPolyLines({(float)ex, (float)ey}, 4, 15, 0, WHITE);
        DrawText(TextFormat("%i HP", enemy.getHP()), ex - 15, ey - 25, 10, RED);
    }

    auto [px, py] = player.getPos();
    DrawCircle(mapOffX + px * 40 + 20, mapOffY + py * 40 + 20, 12, GOLD);

    DrawRectangle(sbX, 0, 1800 - sbX, 1000, {25, 25, 30, 255});
    DrawLine(sbX, 0, sbX, 1000, GOLD);

    DrawText("STATUS CAMPANIE", sbX + 20, 15, 18, SKYBLUE);
    int curDay = clock.getDay();
    DrawText(TextFormat("Ziua: %i / %i", curDay, MAX_DAYS), sbX + 20, 40, 22, (curDay > 40 ? RED : WHITE));
    DrawRectangle(sbX + 20, 68, 360, 4, DARKGRAY);
    float timePerc = (float)curDay / (float)MAX_DAYS;
    DrawRectangle(sbX + 20, 68, (int)(360 * timePerc), 4, SKYBLUE);

    DrawLine(sbX + 20, 85, sbX + 380, 85, DARKGRAY);

    DrawText("FINANTE IMPERIU", sbX + 20, 100, 20, GOLD);
    DrawText(TextFormat("Tezaur: %i Aur", player.getGold()), sbX + 20, 125, 22, WHITE);

    int income = calculateTotalIncome();
    int upkeep = calculateTotalUpkeep();
    int net = income - upkeep;

    DrawText(TextFormat("Profit Net: %+i / zi", net), sbX + 20, 155, 18, (net >= 0 ? GREEN : RED));
    DrawText(TextFormat("Venit (Taxe): +%i", income), sbX + 30, 175, 15, GRAY);
    DrawText(TextFormat("Cheltuieli (Upkeep): -%i", upkeep), sbX + 30, 195, 15, MAROON);

    DrawLine(sbX + 20, 220, sbX + 380, 220, DARKGRAY);

    auto& selRegion = regions[activeRegionIdx];
    City& selCity = selRegion.getCity();
    DrawText(selCity.getName().c_str(), sbX + 20, 235, 24, GOLD);

    if (selCity.isOccupied()) {
        int pop = selCity.getPopulation();
        int lvl = selCity.getCityLevel();
        int upCost = selCity.getUpgradeCost();

        DrawText(TextFormat("Pop.: %i | NIVEL: %i", pop, lvl), sbX + 20, 270, 17, LIGHTGRAY);

        if (lvl < 5) {
            Color costColor = (player.getGold() >= upCost) ? GREEN : RED;
            DrawText(TextFormat("[U] Upgrade: %i Aur", upCost), sbX + 20, 295, 16, costColor);
        } else {
            DrawText("NIVEL MAXIM ATINS", sbX + 20, 295, 16, GOLD);
        }

        int gCount = (int)selCity.getGarrison().size();
        int needed = pop / 100; if (needed < 1) needed = 1;
        float risk = (gCount < needed) ? (float)(needed - gCount) / (float)needed : 0.0f;

        Color riskColor = (risk > 0.5f) ? RED : (risk > 0.0f ? ORANGE : GREEN);
        DrawText(TextFormat("RISC REVOLTA: %i%%", (int)(risk * 100)), sbX + 20, 320, 16, riskColor);

        DrawRectangle(sbX + 20, 340, 300, 8, DARKGRAY);
        if (risk > 0) DrawRectangle(sbX + 20, 340, (int)(300 * risk), 8, riskColor);

        DrawText("GARNIZOANA TA:", sbX + 20, 355, 16, SKYBLUE);
        auto gCounts = selCity.getGarrisonCounts();
        int gy = 380;
        if (gCounts.empty()) DrawText("- Goala (Pericol!) -", sbX + 40, gy, 16, RED);
        for (auto const& [name, count] : gCounts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, WHITE);
            gy += 22;
        }
    } else {
        DrawText("STARE: OCUPAT DE INAMICI", sbX + 20, 270, 17, MAROON);
        DrawText("GARDA DETECTATA:", sbX + 20, 295, 16, RED);
        auto eCounts = selRegion.getEnemyGarrison().getUnitCounts();
        int gy = 320;
        for (auto const& [name, count] : eCounts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, ORANGE);
            gy += 22;
        }
    }

    DrawLine(sbX + 20, 490, sbX + 380, 490, DARKGRAY);
    DrawText("INSPECTIE TINTA", sbX + 20, 500, 20, RED);

    int nextSectionY = 620;

    if (targetedEnemy) {
        DrawText(TextFormat("Inamic la [%i, %i]", targetedEnemy->getX(), targetedEnemy->getY()), sbX + 20, 600, 15, LIGHTGRAY);
        DrawText(TextFormat("HP: %i | ATK: %i", targetedEnemy->getHP(), targetedEnemy->getAtk()), sbX + 20, 545, 17, WHITE);

        auto tCounts = targetedEnemy->getTroops().getUnitCounts();
        int ty = 570;
        for (auto const& [name, count] : tCounts) {
            if (ty > 650) break;
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, ty, 15, RAYWHITE);
            ty += 18;
        }
        DrawText("[F] ATAC", sbX + 250, 500, 16, YELLOW);
        nextSectionY = 630;
    } else {
        DrawText("Nicio amenintare.", sbX + 20, 525, 16, GRAY);
        nextSectionY = 560;
    }

    DrawLine(sbX + 20, nextSectionY, sbX + 380, nextSectionY, DARKGRAY);
    DrawText("ARMATA TA MOBILA", sbX + 20, nextSectionY + 15, 20, GOLD);

    auto pCounts = player.getArmy().getUnitCounts();
    int py2 = nextSectionY + 45;

    if (pCounts.empty() || player.getArmy().isEmpty()) {
        DrawText("- Fara soldati -", sbX + 40, py2, 16, GRAY);
    } else {
        for (auto const& [name, count] : pCounts) {
            if (py2 > 800) {
                DrawText("...", sbX + 40, py2, 16, GRAY);
                break;
            }
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, py2, 16, WHITE);
            py2 += 22;
        }
    }

    DrawRectangle(sbX + 15, 815, 370, 175, {20, 20, 20, 200});
    DrawRectangleLines(sbX + 15, 815, 370, 175, DARKGRAY);
    DrawText("WAR JOURNAL", sbX + 20, 825, 18, GOLD);

    auto allMessages = logger.getMessages();
    int logY = 855;
    int mCount = 0;
    for (auto it = allMessages.rbegin(); it != allMessages.rend(); ++it) {
        if (mCount >= 6) break;
        std::string displayMsg = *it;
        if (displayMsg.length() > 52) {
            displayMsg.resize(49);
            displayMsg += "...";
        }

        Color msgColor = (mCount == 0) ? RAYWHITE : (mCount < 3 ? LIGHTGRAY : GRAY);
        DrawText(displayMsg.c_str(), sbX + 25, logY, 13, msgColor);
        logY += 19;
        mCount++;
    }

    if (showRecruitment) {
        DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 230});
        DrawRectangle(300, 200, 1000, 600, {30, 30, 35, 255});
        DrawRectangleLines(300, 200, 1000, 600, GOLD);
        DrawText("TABARA DE RECRUTARE", 630, 230, 30, GOLD);

        int ry = 320;

        auto drawRow = [&](const char* key, UnitType type) {
            const auto& s = GameData[type];
            DrawText(TextFormat("[%s] %s", key, s.name.c_str()), 350, ry, 22, WHITE);
            DrawText(TextFormat("Atk: %i | HP: %i", s.atk, s.hp), 580, ry, 20, LIGHTGRAY);
            DrawText(TextFormat("Upkeep: %i / zi", s.upkeep), 800, ry, 20, ORANGE);

            Color costColor = (player.getGold() >= s.cost) ? GREEN : RED;
            DrawText(TextFormat("Cost: %i", s.cost), 1100, ry, 20, costColor);
            ry += 70;
        };

        drawRow("1", UnitType::INFANTERIE);
        drawRow("2", UnitType::ARCASI);
        drawRow("3", UnitType::CAVALERIE);
        drawRow("4", UnitType::GARDA);
        drawRow("5", UnitType::EROU);

        DrawText("APASA [R] PENTRU A INCHIDE MENIUL", 620, 750, 18, GRAY);
    }

    DrawRectangle(0, 930, sbX, 70, {15, 15, 20, 220});
    DrawText("[W,A,S,D] Navigare | [N] Zi Noua | [F] Lupta | [R] Recrutare | [TAB] Selectie", 30, 945, 18, RAYWHITE);
    DrawText("[G] Depozitare Garnizoana | [T] Retragere Unitati | [U] Modernizare Oras", 30, 970, 16, GOLD);

    if (showGarrisonMenu) {
        DrawRectangle(450, 350, 350, 250, {20, 20, 25, 245});
        DrawRectangleLines(450, 350, 350, 250, GOLD);

        DrawText("LASA IN GARNIZOANA:", 470, 370, 20, GOLD);
        DrawText("1. Infanterie", 490, 410, 18, WHITE);
        DrawText("2. Arcasi", 490, 440, 18, WHITE);
        DrawText("3. Cavalerie", 490, 470, 18, WHITE);
        DrawText("4. Garda", 490, 500, 18, WHITE);
        DrawText("5. Erou", 490, 530, 18, WHITE);

        DrawText("Apasa [G] pentru a inchide", 470, 560, 15, GRAY);
    }

    if (showTakeMenu) {
        DrawRectangle(450, 350, 350, 250, {25, 25, 30, 245});
        DrawRectangleLines(450, 350, 350, 250, SKYBLUE);

        DrawText("RETRAGE IN ARMATA:", 470, 370, 20, SKYBLUE);
        DrawText("1. Infanterie", 490, 410, 18, WHITE);
        DrawText("2. Arcasi", 490, 440, 18, WHITE);
        DrawText("3. Cavalerie", 490, 470, 18, WHITE);
        DrawText("4. Garda", 490, 500, 18, WHITE);
        DrawText("5. Erou", 490, 530, 18, WHITE);

        DrawText("Apasa [T] pentru a inchide", 470, 560, 15, GRAY);
    }
}

// Bucla principală
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
        window.ClearBackground({30, 30, 35, 255});

        if (currentState == GameState::LOGIN) {
            login.draw();
        } else if (currentState == GameState::SIMULATION) {
            drawUI();
        } else if (currentState == GameState::VICTORIE) {
            DrawRectangle(0, 0, 1800, 1000, {0, 100, 0, 200});
            DrawText("VICTORIE SUPREMA!", 600, 450, 50, GOLD);
            DrawText(TextFormat("Ai cucerit imperiul in %i zile.", clock.getDay()), 620, 520, 20, WHITE);
        } else if (currentState == GameState::DEFEAT) {
            DrawRectangle(0, 0, 1800, 1000, {100, 0, 0, 200});
            DrawText("INFRANGERE!", 650, 450, 50, RED);
            DrawText("Timpul a expirat sau ai pierdut toate cetatile.", 580, 520, 20, WHITE);
        }

        EndDrawing();
    }
}