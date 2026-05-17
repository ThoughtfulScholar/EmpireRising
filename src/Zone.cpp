// #include "Zone.h"
#include "../include/Zone.h"

Zone::Zone(std::string name) : zoneName(std::move(name)) {}

void Zone::addCity(const City& c) {
    cities.push_back(c);
}

bool Zone::resolveBattle(ArmyManager& playerArmy, City& targetCity, GameEngine::Logger& logger) {
    logger.add("--- INCEPE BATALIA PENTRU CETATEA: " + targetCity.getName() + " ---");
    
    ArmyManager& enemyGarrison = targetCity.getGarrison();
    int runda = 1;

    while (!playerArmy.isEmpty() && !enemyGarrison.isEmpty()) {
        logger.add("Runda " + std::to_string(runda) + ":");
        
        Unit* pUnit = playerArmy.getFrontUnit();
        Unit* eUnit = enemyGarrison.getFrontUnit();

        if (pUnit && eUnit) {
            // Jucătorul atacă inamicul
            int pAtk = pUnit->calculateTotalAttack();
            int pDmg = GameEngine::RandomGen::GetInt(pAtk / 2, pAtk);
            eUnit->takeDamage(pDmg);
            logger.add(" -> " + pUnit->getName() + " (Jucator) ataca si da " + std::to_string(pDmg) + " dmg lui " + eUnit->getName());

            // Inamicul ripostează dacă a supraviețuit
            if (!eUnit->isDead()) {
                int eAtk = eUnit->calculateTotalAttack();
                int eDmg = GameEngine::RandomGen::GetInt(eAtk / 2, eAtk);
                pUnit->takeDamage(eDmg);
                logger.add(" <- " + eUnit->getName() + " (Inamic) riposteaza si da " + std::to_string(eDmg) + " dmg lui " + pUnit->getName());
            }
        }

        // Curățare unități căzute la datorie
        playerArmy.removeDeadUnits();
        enemyGarrison.removeDeadUnits();
        runda++;

        if (runda > 100) { // Mecanism de siguranță împotriva buclelor infinite
            logger.add("Batalia s-a blocat intr-un impas de asediu prelungit!");
            break;
        }
    }

    if (enemyGarrison.isEmpty() && !playerArmy.isEmpty()) {
        logger.add("VICTORIE! " + targetCity.getName() + " a fost eliberata de fortele tale!");
        targetCity.setOccupied(true);
        return true;
    } else {
        logger.add("INFRANGERE! Armata ta a fost respinsa de la zidurile cetatii " + targetCity.getName());
        return false;
    }
}

const std::string& Zone::getName() const { return zoneName; }
const std::vector<City>& Zone::getCities() const { return cities; }
std::vector<City>& Zone::getCities() { return cities; }