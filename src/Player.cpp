#include "../include/Player.hpp"

Player::Player(std::string n, int startGold) 
    : name(std::move(n)), gold(startGold), posX(2), posY(2) {}

void Player::removeGold(int amount) {
    if (gold < amount) throw InsufficientGoldException();
    gold -= amount;
}

void Player::move(int dx, int dy, const WorldMap& wm) {
    int nextX = posX + dx;
    int nextY = posY + dy;

    if (wm.isValidMove(nextX, nextY)) {
        posX = nextX;
        posY = nextY;
    } else {
        throw InvalidMovementException("Terenul este impracticabil!");
    }
}

int Player::getUnitLimit(const std::vector<Zone>& regions) const {
    int occupiedCities = 0;
    for (const auto& r : regions) {
        if (r.getCity().isOccupied()) occupiedCities++;
    }
    // Formula: 3 unități de bază + 2 pentru fiecare oraș controlat
    return 3 + (occupiedCities * 2);
}

void Player::recruit(std::unique_ptr<Unit> u, int cost, int limit) {
    if (gold < cost) throw InsufficientGoldException();
    
    if ((int)army.getUnits().size() >= limit) {
        throw PopulationLimitException("Limita de populatie atinsa! Cucereste mai multe cetati.");
    }

    gold -= cost;
    army.addUnit(std::move(u));
}
