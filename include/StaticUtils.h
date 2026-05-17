#ifndef STATICUTILS_H
#define STATICUTILS_H

#include "Exceptions.h"
#include <random>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

namespace GameEngine {

    class RandomGen {
    private:
        static std::mt19937& getEngine() {
            static std::random_device rd;
            static std::mt19937 engine(rd());
            return engine;
        }
    public:
        static int GetInt(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(getEngine());
        }
    };

    class Logger {
    private:
        std::vector<std::string> messages;
        static int totalLogEntries; // Rămâne doar declararea aici

    public:
        void add(const std::string& msg); // Tăiem corpul, punem punct și virgulă
        void logError(const std::exception& e); // Tăiem corpul, punem punct și virgulă

        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
    };
}

// namespace GameEngine {
//     // Generatorul de numere aleatoare
//     class RandomGen {
//     private:
//         static std::mt19937 engine;
//     public:
//         static int GetInt(int min, int max);
//     };
//
//     // Loggerul centralizat pentru jurnalul de război
//     class Logger {
//     private:
//         std::vector<std::string> messages;
//     public:
//         void add(const std::string& msg);
//         void logError(const EmpireException& e);
//         [[nodiscard]] const std::vector<std::string>& getMessages() const;
//     };
// }

// Clasa care gestionează trecerea timpului și taxele globale
class WorldClock {
private:
    int currentDay;
    static float globalTaxModifier; // Rămâne doar declararea aici

public:
    explicit WorldClock() : currentDay(1) {}

    void nextDay() { currentDay++; }
    [[nodiscard]] int getDay() const { return currentDay; }

    static void SetGlobalTaxRate(float rate) { globalTaxModifier = rate; }
    static float GetGlobalTaxRate() { return globalTaxModifier; }
};

#endif // STATICUTILS_H