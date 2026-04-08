#include "EmpireEngine.hpp"
#include <iostream>

int main() {
    try {
        EmpireEngine engine;
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << "EROARE FATALA: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}