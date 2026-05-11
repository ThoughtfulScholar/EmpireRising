#include "../include/Simulation.hpp"
#include "../include/UnitFactory.hpp"
#include "../include/Exceptions.hpp"
#include <algorithm>

void Simulation::handleInput() {
    // 1. GESTIONARE MENIURI ACTIVE (R, G, T)
    if (showRecruitment || showGarrisonMenu || showTakeMenu) {
        if (IsKeyPressed(KEY_R) && showRecruitment) { showRecruitment = false; return; }
        if (IsKeyPressed(KEY_G) && showGarrisonMenu) { showGarrisonMenu = false; return; }
        if (IsKeyPressed(KEY_T) && showTakeMenu) { showTakeMenu = false; return; }

        UnitType selectedType;
        bool keyHit = false;
        if (IsKeyPressed(KEY_ONE))   { selectedType = UnitType::INFANTERIE; keyHit = true; }
        else if (IsKeyPressed(KEY_TWO))   { selectedType = UnitType::ARCASI;     keyHit = true; }
        else if (IsKeyPressed(KEY_THREE)) { selectedType = UnitType::CAVALERIE;  keyHit = true; }
        else if (IsKeyPressed(KEY_FOUR))  { selectedType = UnitType::GARDA;      keyHit = true; }
        else if (IsKeyPressed(KEY_FIVE))  { selectedType = UnitType::EROU;       keyHit = true; }

        if (keyHit) {
            try {
                auto& city = regions[activeRegionIdx].getCity();
                std::string targetName = GameData[selectedType].name;

                if (showGarrisonMenu) {
                    auto& pUnits = player.getArmy().getUnits();
                    auto it = std::find_if(pUnits.begin(), pUnits.end(), [&](const auto& u) {
                        return u->getName() == targetName;
                    });
                    if (it != pUnits.end()) {
                        city.addUnitToGarrison(std::move(*it));
                        pUnits.erase(it);
                        logger.add("Transferat in garnizoana: " + targetName);
                    } else throw EmpireException("Nu ai aceasta unitate in armata!");
                } 
                else if (showTakeMenu) {
                    int limit = player.getUnitLimit(regions);
                    if ((int)player.getArmy().getUnits().size() >= limit) throw PopulationLimitException("Armata plina!");
                    
                    auto& gUnits = city.getGarrison();
                    auto it = std::find_if(gUnits.begin(), gUnits.end(), [&](const auto& u) {
                        return u->getName() == targetName;
                    });
                    if (it != gUnits.end()) {
                        player.getArmy().addUnit(std::move(*it));
                        gUnits.erase(it);
                        logger.add("Recuperat din oras: " + targetName);
                    } else throw EmpireException("Unitatea nu exista in garnizoana!");
                }
                else if (showRecruitment) {
                    player.recruit(UnitFactory::CreateUnit(selectedType), GameData[selectedType].cost, player.getUnitLimit(regions));
                    logger.add("Recrutat: " + targetName);
                }
            } catch (const EmpireException& e) { logger.logError(e); }
        }
        return; 
    }

    // 2. NAVIGARE CU TRY-CATCH
    auto [px, py] = player.getPos();
    int dx = 0, dy = 0;
    if (IsKeyPressed(KEY_W)) dy = -1;
    else if (IsKeyPressed(KEY_S)) dy = 1;
    else if (IsKeyPressed(KEY_A)) dx = -1;
    else if (IsKeyPressed(KEY_D)) dx = 1;

    if (dx != 0 || dy != 0) {
        try {
            player.move(dx, dy, worldMap);
        } catch (const EmpireException& e) { logger.logError(e); }
    }

    // 3. COMBAT ȘI INTERACȚIUNE [F]
    if (IsKeyPressed(KEY_F)) {
        // Căutare inamic adiacent
        targetedEnemy = nullptr;
        for (auto& enemy : roamingEnemies) {
            if (std::abs(enemy.getX() - px) <= 1 && std::abs(enemy.getY() - py) <= 1) {
                targetedEnemy = &enemy; break;
            }
        }

        if (targetedEnemy) {
            try {
                if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe!");
                int pAtk = 0;
                for(const auto& u : player.getArmy().getUnits()) pAtk += u->calculateTotalAttack();
                targetedEnemy->takeDamage(pAtk);
                if (targetedEnemy->isDefeated()) {
                    player.earnGold(250); logger.add("VICTORIE! Armata inamica distrusa.");
                } else {
                    player.getArmy().getFrontUnit()->takeDamage(targetedEnemy->getAtk());
                    player.getArmy().removeDeadUnits();
                }
            } catch (const EmpireException& e) { logger.logError(e); }
        } else {
            // Verificare asediu oraș
            auto& sel = regions[activeRegionIdx];
            auto [cx, cy] = sel.getCity().getPos();
            if (px == cx && py == cy) {
                try { logger.add(sel.executeBattleRound(player.getArmy())); }
                catch (const EmpireException& e) { logger.logError(e); }
            }
        }
    }

    // 4. ALTE COMENZI
    if (IsKeyPressed(KEY_TAB)) activeRegionIdx = (activeRegionIdx + 1) % regions.size();
    if (IsKeyPressed(KEY_N)) processNextDay();
    if (IsKeyPressed(KEY_R)) showRecruitment = true;
    
    // Auto-selectare oraș sub jucător
    for (int i = 0; i < (int)regions.size(); i++) {
        auto [cx, cy] = regions[i].getCity().getPos();
        if (px == cx && py == cy) {
            activeRegionIdx = i;
            if (regions[i].getCity().isOccupied()) {
                if (IsKeyPressed(KEY_G)) showGarrisonMenu = true;
                if (IsKeyPressed(KEY_T)) showTakeMenu = true;
                if (IsKeyPressed(KEY_U)) {
                    City& c = regions[i].getCity();
                    try {
                        player.removeGold(c.getUpgradeCost());
                        c.upgrade();
                        logger.add("Modernizat: " + c.getName());
                    } catch (const EmpireException& e) { logger.logError(e); }
                }
            }
            break;
        }
    }
}
