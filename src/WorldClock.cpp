#include "../include/WorldClock.hpp"

// Inițializarea atributului static
float WorldClock::globalTaxRate = 1.0f;

WorldClock::WorldClock() : day(1) {}

void WorldClock::nextDay() {
    day++;
    // Putem adăuga logică de fluctuație a taxelor în funcție de evenimente aici
}

std::ostream& operator<<(std::ostream& os, const WorldClock& wc) {
    os << "Ziua " << wc.day << " (Taxe: " << (int)(WorldClock::globalTaxRate * 100) << "%)";
    return os;
}
