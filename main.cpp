#include "include/Simulation.hpp"
#include <iostream>

int main() {
    try {
        Simulation game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "EROARE CRITICA: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
