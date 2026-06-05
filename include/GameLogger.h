// include/GameLogger.h
#ifndef GAMELOGGER_H
#define GAMELOGGER_H

#include <iostream>
#include <string>
#include <vector>

class GameLogger {
private:
    std::vector<std::string> logs;

    // Constructor privat (specific Singleton)
    GameLogger() = default;

public:
    // Ștergem constructorul de copiere și operatorul= pentru a preveni duplicarea
    GameLogger(const GameLogger&) = delete;
    GameLogger& operator=(const GameLogger&) = delete;

    // Punctul unic de acces la instanță
    static GameLogger& getInstance() {
        static GameLogger instance; // Thread-safe în C++11+
        return instance;
    }

    void logEvent(const std::string& message) {
        logs.push_back(message);
        std::cout << "[LOG]: " << message << "\n";
    }

    const std::vector<std::string>& getHistory() const { return logs; }
};

#endif // GAMELOGGER_H