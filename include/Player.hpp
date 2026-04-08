#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>
#include <memory>
#include <string>
#include "Unit.hpp"

class Player {
private:
    std::string name;
    int gold;
    int wood;
    std::vector<std::unique_ptr<Unit>> army;

public:
    // Tema 1: Constructor cu valori default si lista de initializare
    explicit Player(std::string n = "Comandant", int g = 1000, int w = 200);

    // --- Tema 2: Rule of Three ---
    Player(const Player& other);            // Copy Constructor (Deep Copy)
    Player& operator=(const Player& other); // Assignment Operator
    ~Player();                              // Destructor

    // Gestiune Resurse
    void addResources(int g, int w);
    bool spendResources(int g, int w);

    // Gestiune Unitati
    void recruit(std::unique_ptr<Unit> unit);

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] int getWood() const { return wood; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getArmy() const { return army; }
};

#endif