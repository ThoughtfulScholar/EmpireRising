#ifndef CITY_HPP
#define CITY_HPP

#include <string>

class City {
private:
    std::string cityName;
    int level;
    int defense;
    int goldProduction;
    bool playerOwned;

public:
    // Tema 1: Constructor cu lista de initializare
    explicit City(std::string name, bool owned = false);

    // Mecanica de Upgrade (Cerința ta)
    void upgrade();

    // Getteri
    [[nodiscard]] std::string getName() const { return cityName; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] int getDefense() const { return defense; }
    [[nodiscard]] int getProduction() const { return goldProduction; }
    [[nodiscard]] bool isPlayerOwned() const { return playerOwned; }

    void setPlayerOwned(bool owned) { playerOwned = owned; }
};

#endif