// #include "StaticUtils.h"
#include "../include/StaticUtils.h"


namespace GameEngine {
    // Inițializare motor aleator static
    std::mt19937 RandomGen::engine(std::random_device{}());

    int RandomGen::GetInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine);
    }

    void Logger::add(const std::string& msg) {
        messages.push_back(msg);
        std::cout << "[LOG] " << msg << "\n";
    }

    void Logger::logError(const EmpireException& e) {
        add("EROARE: " + std::string(e.what()));
    }

    const std::vector<std::string>& Logger::getMessages() const {
        return messages;
    }
}

// Inițializare membru static al clasei WorldClock
float WorldClock::GlobalTaxRate = 1.0f;

void WorldClock::nextDay() {
    currentDay++;
}

int WorldClock::getDay() const {
    return currentDay;
}

void WorldClock::SetGlobalTaxRate(float rate) {
    GlobalTaxRate = rate;
}

float WorldClock::GetGlobalTaxRate() {
    return GlobalTaxRate;
}