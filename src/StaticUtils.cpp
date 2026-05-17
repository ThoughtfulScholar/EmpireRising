// #include "StaticUtils.h"
#include "../include/StaticUtils.h"


namespace GameEngine {

    // Inițializarea atributului static se mută obligatoriu aici, în .cpp
    int Logger::totalLogEntries = 0;

    void Logger::add(const std::string& msg) {
        messages.push_back(msg);
        totalLogEntries++;
        if (messages.size() > 10) {
            messages.erase(messages.begin());
        }
    }

    void Logger::logError(const std::exception& e) {
        add("![EROARE]: " + std::string(e.what()));
    }

} // namespace GameEngine

// Inițializare membru static al clasei WorldClock
float WorldClock::globalTaxModifier = 1.0f;