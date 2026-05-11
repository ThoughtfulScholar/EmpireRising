#include "../include/Simulation.hpp"
#include "../include/UnitFactory.hpp"
#include <cmath>
#include <algorithm>

Simulation::Simulation() 
    : window(1600, 1000, "EMPIRE RISING - TEMA 2"), 
      currentState(GameState::LOGIN), 
      worldMap(30, 20) {
    SetTargetFPS(60);
}

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

void Simulation::initWorld() {
    // 1. Relief Natural
    for (int i = 0; i < 40; ++i) {
        int tx = GameEngine::RandomGen::GetInt(0, worldMap.getWidth() - 1);
        int ty = GameEngine::RandomGen::GetInt(0, worldMap.getHeight() - 1);
        int roll = GameEngine::RandomGen::GetInt(0, 100);
        
        if (roll < 15) worldMap.setTile(tx, ty, TerrainType::MOUNTAIN);
        else if (roll < 30) worldMap.setTile(tx, ty, TerrainType::FOREST);
        else if (roll < 40) worldMap.setTile(tx, ty, TerrainType::WATER);
    }

    // 2. Orașe și Drumuri
    for (int i = 0; i < 6; ++i) {
        int cx = GameEngine::RandomGen::GetInt(2, 27);
        int cy = GameEngine::RandomGen::GetInt(2, 17);

        worldMap.setTile(cx, cy, TerrainType::CITY_TILE);
        worldMap.createPath(2, 2, cx, cy);

        City c((i == 0 ? "Capitala" : "Cetate " + std::to_string(i)), cx, cy);
        if (i == 0) {
            c.setOccupied(true);
            regions.emplace_back("Provincia Natala", std::move(c), GREEN);
        } else {
            regions.emplace_back("Tinut Inamic", std::move(c), RED);
        }
    }
}

void Simulation::processNextDay() {
    if (gameOver) return;

    clock.nextDay();
    logger.add("--- ZIUA " + std::to_string(clock.getDay()) + " ---");

    int occupiedCitiesCount = 0;
    int currentIncome = 0;
    int currentUpkeep = player.getArmy().calculateTotalUpkeep();

    for (auto& reg : regions) {
        City& city = reg.getCity();
        if (city.isOccupied()) {
            occupiedCitiesCount++;
            city.growPopulation();
            currentIncome += (int)(city.collectTaxes() * WorldClock::GetGlobalTaxRate());
            currentUpkeep += city.getGarrisonUpkeep();
            city.checkRebellion(logger);
        }
    }

    int netProfit = currentIncome - currentUpkeep;
    player.earnGold(netProfit);
    logger.add(TextFormat("Economie: Net %+i Aur", netProfit));

    // Mișcare Inamici
    for (auto& enemy : roamingEnemies) {
        Zone* targetZone = nullptr;
        float minDistance = 999.0f;

        for (auto& reg : regions) {
            if (reg.getCity().isOccupied()) {
                auto [cx, cy] = reg.getCity().getPos();
                float dist = (float)std::sqrt(std::pow(cx - enemy.getX(), 2) + std::pow(cy - enemy.getY(), 2));
                if (dist < minDistance) { minDistance = dist; targetZone = &reg; }
            }
        }

        if (targetZone) {
            auto [tx, ty] = targetZone->getCity().getPos();
            if (minDistance < 1.5f) {
                City& targetCity = targetZone->getCity();
                if (targetCity.getGarrison().empty()) {
                    targetCity.setOccupied(false);
                    logger.add("!! " + targetCity.getName() + " a cazut!");
                } else {
                    targetCity.extractUnit();
                    enemy.takeDamage(100);
                    logger.add("Garnizoana din " + targetCity.getName() + " a rezistat.");
                }
            } else {
                enemy.moveTowards(tx, ty, worldMap);
            }
        }
    }

    // Spawn Inamici
    if (clock.getDay() % 6 == 0) {
        roamingEnemies.emplace_back(29, GameEngine::RandomGen::GetInt(0, 19), 2 + (clock.getDay() / 10));
        logger.add("ALERTA: Intruziune inamica!");
    }

    std::erase_if(roamingEnemies, [](const EnemyArmy& e) { return e.isDefeated(); });

    // Win/Loss Checks
    if (occupiedCitiesCount == (int)regions.size()) { currentState = GameState::VICTORIE; gameOver = true; }
    else if (occupiedCitiesCount <= 0 && clock.getDay() > 1) { currentState = GameState::DEFEAT; gameOver = true; }
    else if (player.getGold() < -1000) { currentState = GameState::DEFEAT; gameOver = true; }
    else if (clock.getDay() >= MAX_DAYS) { currentState = GameState::DEFEAT; gameOver = true; }
}
