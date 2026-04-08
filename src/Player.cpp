#include "Player.hpp"
#include <utility>

Player::Player(std::string n, int g, int w)
    : name(std::move(n)), gold(g), wood(w) {}

// Copy Constructor: Realizam Deep Copy manual
Player::Player(const Player& other)
    : name(other.name), gold(other.gold), wood(other.wood) {
    for (const auto& u : other.army) {
        // Folosim tipul unitatii pentru a crea o copie noua (Clonare manuala)
        auto [x, y] = u->getPosition();
        if (u->getType() == UnitType::INFANTRY)
            recruit(std::make_unique<Infantry>(u->getName(), x, y));
        else if (u->getType() == UnitType::CAVALRY)
            recruit(std::make_unique<Cavalry>(u->getName(), x, y));
        else if (u->getType() == UnitType::ARCHER)
            recruit(std::make_unique<Archer>(u->getName(), x, y));
    }
}

// Assignment Operator: Eliberam resursele vechi si copiem cele noi
Player& Player::operator=(const Player& other) {
    if (this != &other) {
        name = other.name;
        gold = other.gold;
        wood = other.wood;
        army.clear(); // unique_ptr sterge automat obiectele vechi
        for (const auto& u : other.army) {
            auto [x, y] = u->getPosition();
            if (u->getType() == UnitType::INFANTRY)
                recruit(std::make_unique<Infantry>(u->getName(), x, y));
            else if (u->getType() == UnitType::CAVALRY)
                recruit(std::make_unique<Cavalry>(u->getName(), x, y));
            else if (u->getType() == UnitType::ARCHER)
                recruit(std::make_unique<Archer>(u->getName(), x, y));
        }
    }
    return *this;
}

// Destructorul (default e suficient deoarece unique_ptr curata tot)
Player::~Player() = default;

void Player::addResources(int g, int w) {
    gold += g;
    wood += w;
}

bool Player::spendResources(int g, int w) {
    if (gold >= g && wood >= w) {
        gold -= g;
        wood -= w;
        return true;
    }
    return false;
}

void Player::recruit(std::unique_ptr<Unit> unit) {
    if (unit) {
        army.push_back(std::move(unit));
    }
}