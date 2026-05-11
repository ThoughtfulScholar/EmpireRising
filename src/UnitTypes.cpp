#include "../include/UnitTypes.hpp"

// --- INFANTRY ---
Infantry::Infantry() : Unit("Infanterie", 300, 45, 25) {}
std::unique_ptr<Unit> Infantry::clone() const { 
    return std::make_unique<Infantry>(*this); 
}

// --- ARCHER ---
Archer::Archer() : Unit("Arcas", 180, 70, 30) {}
std::unique_ptr<Unit> Archer::clone() const { 
    return std::make_unique<Archer>(*this); 
}

// --- CAVALRY ---
Cavalry::Cavalry() : Unit("Cavalerie", 350, 60, 50) {}
std::unique_ptr<Unit> Cavalry::clone() const { 
    return std::make_unique<Cavalry>(*this); 
}

// --- GARRISON GUARD ---
GarrisonGuard::GarrisonGuard(const std::string& n) : Unit(n, 250, 40, 15) {}
std::unique_ptr<Unit> GarrisonGuard::clone() const { 
    return std::make_unique<GarrisonGuard>(*this); 
}

// --- HERO ---
Hero::Hero(const std::string& n, int h, int a, int u) : Unit(n, h, a, u) {}
std::unique_ptr<Unit> Hero::clone() const { 
    return std::make_unique<Hero>(*this); 
}

// --- GARRISON UNIT ---
GarrisonUnit::GarrisonUnit() : Unit("Garnizoana", 200, 30, 10) {}
std::unique_ptr<Unit> GarrisonUnit::clone() const { 
    return std::make_unique<GarrisonUnit>(*this); 
}
