#include "../include/City.hpp"
#include <algorithm>

City::City(std::string n, int x, int y, int pop)
    : name(std::move(n)), posX(x), posY(y), population(pop), cityLevel(1), occupied(false) {}

City::City(const City& other)
    : name(other.name), posX(other.posX), posY(other.posY),
      population(other.population), cityLevel(other.cityLevel), occupied(other.occupied) {
    for (const auto& u : other.garrison) {
        if (u) garrison.push_back(u->clone());
    }
}

City& City::operator=(const City& other) {
    if (this != &other) {
        name = other.name; posX = other.posX; posY = other.posY;
        population = other.population; cityLevel = other.cityLevel;
        occupied = other.occupied;
        garrison.clear();
        for (const auto& u : other.garrison) {
            if (u) garrison.push_back(u->clone());
        }
    }
    return *this;
}

void City::upgrade() {
    cityLevel++;
    population += 100;
}

void City::growPopulation() {
    if (occupied) population += (20 * cityLevel);
}

int City::collectTaxes() const {
    if (!occupied) return 0;
    return (population / 2) * cityLevel;
}

int City::getGarrisonUpkeep() const {
    int total = 0;
    for (const auto& u : garrison) {
        if (u) total += u->getUpkeep();
    }
    return total;
}

void City::addUnitToGarrison(std::unique_ptr<Unit> u) {
    if (u) {
        garrison.push_back(std::move(u));
        occupied = true;
    }
}

std::unique_ptr<Unit> City::extractUnit() {
    if (garrison.empty()) return nullptr;
    auto u = std::move(garrison.back());
    garrison.pop_back();
    return u;
}

std::string City::getName() const {
    std::string idOnly = "";
    for (char c : this->name) if (isdigit(c)) idOnly += c;

    if (idOnly.empty()) return this->name;
    return (this->occupied ? "Cetate Aliata " : "Cetate Inamica ") + idOnly;
}

std::map<std::string, int> City::getGarrisonCounts() const {
    std::map<std::string, int> counts;
    for (const auto& u : garrison) if (u) counts[u->getName()]++;
    return counts;
}

std::ostream& operator<<(std::ostream& os, const City& c) {
    os << "Oras: " << c.name << " (Pop: " << c.population << ")\n";
    os << "Garnizoana: ";
    for(const auto& u : c.garrison) os << *u << " | ";
    return os;
}
