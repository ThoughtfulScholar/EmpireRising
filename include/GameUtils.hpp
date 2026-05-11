#ifndef GAME_UTILS_HPP
#define GAME_UTILS_HPP

#include <vector>
#include <string>
#include <random>
#include <exception>

namespace GameEngine {
    class RandomGen {
    public:
        // Returnează un număr întreg între min și max
        static int GetInt(int min, int max);
    };

    class Logger {
    private:
        std::vector<std::string> messages;
        static int totalLogEntries; // Atribut static (Cerință Tema 2)

    public:
        void add(const std::string& msg);
        void logError(const std::exception& e);
        
        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
        static int getTotalLogs() { return totalLogEntries; }
    };
}

#endif
