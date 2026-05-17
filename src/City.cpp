#include "../include/City.h"
#include <raylib.h> // Pentru GetRandomValue utilizat în logica din .h, adăugat preventiv aici
#include <cctype>   // Pentru isdigit

// Constructor principal
City::City(std::string n, int x, int y, int pop)
    : name(std::move(n)), posX(x), posY(y), population(pop), cityLevel(1), occupied(false) {}

// RULE OF THREE: Copy Constructor
City::City(const City& other)
    : name(other.name), posX(other.posX), posY(other.posY),
      population(other.population), cityLevel(other.cityLevel), occupied(other.occupied) {
    for (const auto& u : other.garrison) {
        if (u) garrison.push_back(u->clone());
    }
}

// RULE OF THREE: Copy Assignment
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

// --- MECANICI DE EVOLUȚIE ---
void City::upgrade() {
    cityLevel++;
    population += 100; // Bonus de expansiune
}

void City::growPopulation() {
    if (occupied) {
        // Populația crește în funcție de stabilitate și nivel
        population += (20 * cityLevel);
    }
}

// --- ECONOMIE (TAXE ȘI SALARII) ---
int City::collectTaxes() const {
    if (!occupied) return 0;
    // Taxe mult mai mari conform cerinței (Populația / 2 * Nivel)
    return (population / 2) * cityLevel;
}

int City::getGarrisonUpkeep() const {
    int total = 0;
    for (const auto& u : garrison) {
        if (u) total += u->getUpkeep();
    }
    return total;
}

// --- GESTIONARE TRUPE ---
void City::addUnitToGarrison(std::unique_ptr<Unit> u) {
    if (u) {
        garrison.push_back(std::move(u));
        occupied = true; // Dacă pui trupe, orașul devine ocupat/al tău
    }
}

std::unique_ptr<Unit> City::extractUnit() {
    if (garrison.empty()) return nullptr;
    auto u = std::move(garrison.back());
    garrison.pop_back();
    return u;
}

std::string City::getName() const {
    // Extragem doar numărul din "Cetate Inamica 1" sau "Cetate Inamica 2"
    // Căutăm ultima cifră din string-ul 'name'
    std::string idOnly = "";
    for (char c : this->name) {
        if (isdigit(c)) {
            idOnly += c;
        }
    }

    // Dacă din vreun motiv nu găsește cifre, returnăm numele original să nu crape
    if (idOnly.empty()) return this->name;

    if (this->occupied) {
        return "Cetate Aliata " + idOnly;
    } else {
        return "Cetate Inamica " + idOnly;
    }
}

std::map<std::string, int> City::getGarrisonCounts() const {
    std::map<std::string, int> counts;
    for (const auto& u : garrison) {
        if (u) counts[u->getName()]++;
    }
    return counts;
}

// Operatorul de afișare pentru City
std::ostream& operator<<(std::ostream& os, const City& c) {
    os << "Oras: " << c.name << " (Pop: " << c.population << ")\n";
    os << "Garnizoana: ";
    for(const auto& u : c.garrison) {
        os << *u << " | ";
    }
    return os;
}