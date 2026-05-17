#include "include/Simulation.h"
#include "include/UnitFactory.h"
#include "include/Exceptions.h"
#include "include/StaticUtils.h"
#include <iostream>
#include <memory>

// Funcția obligatorie pentru demonstrarea cerințelor academice în consolă
void RunRequirementsDemo() {
    std::cout << ">>> DEMO TEMA 2: VERIFICARE CERINTE <<<\n";

    // 1. Polimorfism & Clone (Virtual Copy Constructor)
    std::unique_ptr<Unit> u1 = UnitFactory::CreateUnit(UnitType::EROU, "Achile");
    auto u2 = u1->clone();
    std::cout << "Original: " << *u1 << "\nClona: " << *u2 << "\n";

    // 2. Verificare RTTI prin downcasting securizat (dynamic_cast)
    if (const auto* h = dynamic_cast<Hero*>(u1.get())) {
        std::cout << "dynamic_cast reusit: " << h->getName() << " este Erou.\n";
    }

    // 3. Verificare variabile și metode statice
    WorldClock::SetGlobalTaxRate(1.2f);
    std::cout << "Taxa globala setata la: " << WorldClock::GetGlobalTaxRate() << "\n";
    std::cout << "Total unitati create in memorie: " << Unit::getTotalUnits() << "\n";

    u1->display(std::cout);
    std::cout << "\n";

    // 4. Mecanismul de tratare a excepțiilor custom (Ierarhie proprie)
    try {
        throw PopulationLimitException("Test limita populatie.");
    } catch (const EmpireException& e) {
        std::cout << "Exceptie prinsa: " << e.what() << "\n";
    }
    
    std::cout << ">>> SFARSIT DEMO. Pornire interfata grafica...\n";
}

int main() {
    // Rulare verificare cerințe în consolă
    RunRequirementsDemo();
    
    // Pornire buclă principală de simulare grafică Raylib
    try {
        Simulation game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "EROARE CRITICA: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}