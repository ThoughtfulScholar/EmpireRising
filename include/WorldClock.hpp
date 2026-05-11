#ifndef WORLD_CLOCK_HPP
#define WORLD_CLOCK_HPP

#include <iostream>

class WorldClock {
private:
    int day;
    static float globalTaxRate; // Atribut static (Cerință Tema 2)

public:
    WorldClock();

    void nextDay();
    [[nodiscard]] int getDay() const { return day; }

    // Metode statice pentru gestionarea economiei globale
    static void SetGlobalTaxRate(float rate) { globalTaxRate = rate; }
    static float GetGlobalTaxRate() { return globalTaxRate; }

    friend std::ostream& operator<<(std::ostream& os, const WorldClock& wc);
};

#endif
