#include "../include/Zone.h"

// Constructorul clasei Zone
Zone::Zone(std::string name, City city, raylib::Color tint)
    : zoneName(std::move(name)), localCity(std::move(city)), mapTint(tint) {

    // Dacă nu este capitala, îi punem o gardă inamică inițială
    if (zoneName != "Provincia Natala") {
        int guards = GameEngine::RandomGen::GetInt(2, 3);
        for(int i = 0; i < guards; ++i) {
            enemyGarrison.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Garda Inamica"));
        }
    }
}

// Metoda de luptă
std::string Zone::executeBattleRound(ArmyManager& playerArmy) {
    if (localCity.isOccupied()) return "Orasul este deja sub controlul tau.";
    if (playerArmy.isEmpty()) throw CombatException("Nu ai trupe pentru asediu!");

    Unit* pUnit = playerArmy.getFrontUnit();
    Unit* eUnit = enemyGarrison.getFrontUnit();

    // 1. Atacul Jucătorului
    int pAtk = pUnit->calculateTotalAttack();
    eUnit->takeDamage(pAtk);
    pUnit->playAttackSound();

    std::string log = pUnit->getName() + " loveste cu " + std::to_string(pAtk) + ". ";

    if (!eUnit->isAlive()) {
        enemyGarrison.removeDeadUnits();
        pUnit->gainXP(50);
        if (enemyGarrison.isEmpty()) {
            localCity.setOccupied(true);
            return log + "VICTORIE! Cetatea a fost cucerita.";
        }
        return log + "Inamic rapus!";
    }

    // 2. Riposta Inamicului
    int eAtk = eUnit->calculateTotalAttack();
    pUnit->takeDamage(eAtk);
    log += "Inamicul riposteaza: -" + std::to_string(eAtk) + " HP.";

    if (!pUnit->isAlive()) {
        playerArmy.removeDeadUnits();
        return log + " Unitatea ta a cazut!";
    }

    return log;
}

// Implementarea operatorului de afișare pentru Zone
std::ostream& operator<<(std::ostream& os, const Zone& z) {
    os << "Regiunea: " << z.zoneName << "\n"
       << z.localCity << "\n";
    if (!z.localCity.isOccupied()) {
        os << "  [Garda Inamica]: " << z.enemyGarrison;
    }
    return os;
}