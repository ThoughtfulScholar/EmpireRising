#include "City.hpp"
#include <utility>

City::City(std::string name, bool owned)
    : cityName(std::move(name)), level(1), defense(100), goldProduction(50), playerOwned(owned) {}

void City::upgrade() {
    level++;
    defense += 150;
    goldProduction += 100;
}