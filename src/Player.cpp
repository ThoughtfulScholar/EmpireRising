#include "../include/Player.h"

// Constructorul clasei Player
Player::Player(std::string name, int startingGold)
    : commanderName(std::move(name)), gold(startingGold), posX(2), posY(2) {}

// Logica statică pentru calculul limitei de populație
int Player::getUnitLimit(const std::vector<Zone>& regions) {
    int limit = 4; // Limită de bază (corturile de campanie)
    for (const auto& r : regions) {
        if (r.getCity().isOccupied()) {
            // Fiecare oraș cucerit oferă +3 sloturi de unități
            limit += 3;
        }
    }
    return limit;
}

// Logica de recrutare trupe
void Player::recruit(std::unique_ptr<Unit> u, int cost, int maxLimit) {
    if (static_cast<int>(army.getUnits().size()) >= maxLimit) {
        throw PopulationLimitException("Limita de populatie atinsa! Cucereste orase pentru mai multe sloturi.");
    }
    if (gold >= cost) {
        gold -= cost;
        army.addUnit(std::move(u));
    } else {
        throw InsufficientGoldException();
    }
}

// Logica de mișcare pe harta lumii
void Player::move(int dx, int dy, const WorldMap& worldMap) {
    int newX = posX + dx;
    int newY = posY + dy;
    if (!worldMap.isValidMove(newX, newY)) {
        throw InvalidMovementException("Teren impracticabil!");
    }
    posX = newX;
    posY = newY;
}

// Implementarea operatorului de afișare pentru Player
std::ostream& operator<<(std::ostream& os, const Player& p) {
    os << "Comandant: " << p.commanderName << " | Tezaur: " << p.gold << " aur\n";
    return os;
}
