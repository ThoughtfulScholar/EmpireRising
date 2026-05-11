#include "../include/GameUtils.hpp"

namespace GameEngine {
    // Inițializare atribut static
    int Logger::totalLogEntries = 0;

    // Implementare RandomGen folosind un motor static pentru eficiență
    int RandomGen::GetInt(int min, int max) {
        static std::random_device rd;
        static std::mt19937 engine(rd());
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine);
    }

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
}
