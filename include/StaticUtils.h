#ifndef STATICUTILS_H
#define STATICUTILS_H

#include "Exceptions.h"
#include <random>
#include <vector>
#include <string>
#include <iostream>

namespace GameEngine {
    // Generatorul de numere aleatoare
    class RandomGen {
    private:
        static std::mt19937 engine;
    public:
        static int GetInt(int min, int max);
    };

    // Loggerul centralizat pentru jurnalul de război
    class Logger {
    private:
        std::vector<std::string> messages;
    public:
        void add(const std::string& msg);
        void logError(const EmpireException& e);
        [[nodiscard]] const std::vector<std::string>& getMessages() const;
    };
}

// Clasa care gestionează trecerea timpului și taxele globale
class WorldClock {
private:
    int currentDay = 1;
    static float GlobalTaxRate;
public:
    void nextDay();
    [[nodiscard]] int getDay() const;
    static void SetGlobalTaxRate(float rate);
    static float GetGlobalTaxRate();
};

#endif // STATICUTILS_H