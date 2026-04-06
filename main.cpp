#include <iostream>
#include <array>
#include <chrono>
#include <thread>

#include <raylib-cpp.hpp>

#include <iostream>
#include <array>
#include "include/Example.h"
// This also works if you do not want `include/`, but some editors might not like it
// #include "Example.h"
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////
/// This class is used to test that the memory leak checks work as expected even when using a GUI
class SomeClass {
class Unit {
private:
    std::string name;
    int health;
    int attack;
public:
    explicit SomeClass(int) {}
    Unit(const std::string& name = "", int health = 0, int attack = 0)
        : name(name), health(health), attack(attack) {}

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool isAlive() const {
        return health > 0;
    }

    int getPower() const {
        return health + attack;
    }

    const std::string& getName() const { return name; }
    int getHealth() const { return health; }
    int getAttack() const { return attack; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit{name=" << u.name
           << ", health=" << u.health
           << ", attack=" << u.attack << "}";
        return os;
    }
};

SomeClass *getC() {
    return new SomeClass{2};
}
//////////////////////////////////////////////////////////////////////
class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> units;
public:
    Player(const std::string& name = "", int gold = 0)
        : name(name), gold(gold), units() {}

    Player(const Player& other)
        : name(other.name), gold(other.gold), units(other.units) {}

int main() {
    std::cout << "Hello, world!\n";
    Example e1;
    e1.g();
    std::array<int, 100> v{};
    int nr;
    std::cout << "Introduceți nr: ";
    /////////////////////////////////////////////////////////////////////////
    /// Observație: dacă aveți nevoie să citiți date de intrare de la tastatură,
    /// dați exemple de date de intrare folosind fișierul tastatura.txt
    /// Trebuie să aveți în fișierul tastatura.txt suficiente date de intrare
    /// (în formatul impus de voi) astfel încât execuția programului să se încheie.
    /// De asemenea, trebuie să adăugați în acest fișier date de intrare
    /// pentru cât mai multe ramuri de execuție.
    /// Dorim să facem acest lucru pentru a automatiza testarea codului, fără să
    /// mai pierdem timp de fiecare dată să introducem de la zero aceleași date de intrare.
    ///
    /// Pe GitHub Actions (bife), fișierul tastatura.txt este folosit
    /// pentru a simula date introduse de la tastatură.
    /// Bifele verifică dacă programul are erori de compilare, erori de memorie și memory leaks.
    ///
    /// Dacă nu puneți în tastatura.txt suficiente date de intrare, îmi rezerv dreptul să vă
    /// testez codul cu ce date de intrare am chef și să nu pun notă dacă găsesc vreun bug.
    /// Impun această cerință ca să învățați să faceți un demo și să arătați părțile din
    /// program care merg (și să le evitați pe cele care nu merg).
    ///
    /////////////////////////////////////////////////////////////////////////
    std::cin >> nr;
    /////////////////////////////////////////////////////////////////////////
    for(int i = 0; i < nr; ++i) {
        std::cout << "v[" << i << "] = ";
        std::cin >> v[i];
    }
    std::cout << "\n\n";
    std::cout << "Am citit de la tastatură " << nr << " elemente:\n";
    for(int i = 0; i < nr; ++i) {
        std::cout << "- " << v[i] << "\n";
    }
    ///////////////////////////////////////////////////////////////////////////
    /// Pentru date citite din fișier, NU folosiți tastatura.txt. Creați-vă voi
    /// alt fișier propriu cu ce alt nume doriți.
    /// Exemplu:
    /// std::ifstream fis("date.txt");
    /// for(int i = 0; i < nr2; ++i)
    ///     fis >> v2[i];
    ///
    ///////////////////////////////////////////////////////////////////////////

    SomeClass *c = getC();
    std::cout << c << "\n";
    delete c;  // comentarea acestui rând ar trebui să ducă la semnalarea unui mem leak

// Dimensiunile ferestrei
    int screenWidth = 800;
    int screenHeight = 700;

    // Crearea ferestrei (constructorul raylib::Window o inițializează automat)
    // NOTE: sync with env variable APP_WINDOW from .github/workflows/cmake.yml:31
    raylib::Window window(screenWidth, screenHeight, "My Window");

    std::cout << "Fereastra a fost creată\n";

    // NOTE: mandatory use one of vsync or FPS limit (not both)
    // În Raylib, setarea FPS limitează automat și consumul GPU
    window.SetTargetFPS(60); 
    // Dacă preferi VSync: SetConfigFlags(FLAG_VSYNC_HINT); înainte de init

    // Loop-ul principal
    while (!window.ShouldClose()) { // ShouldClose verifică automat butonul de închidere (X)
        bool shouldExit = false;

        // Gestionarea evenimentelor (Raylib verifică starea în fiecare frame)
        
        // Verificare Resize
        if (window.IsResized()) {
            std::cout << "New width: " << window.GetWidth() << '\n'
                      << "New height: " << window.GetHeight() << '\n';
    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            units = other.units;
        }
        return *this;
    }

    ~Player() = default;

    void addUnit(const Unit& u) {
        units.push_back(u);
    }

    int getTotalPower() const {
        int total = 0;
        for (const auto& u : units) {
            total += u.getPower();
}
        return total;
    }

    bool spendGold(int amount) {
        if (amount > gold) return false;
        gold -= amount;
        return true;
    }

        // Verificare Taste
        if (IsKeyPressed(KEY_X)) {
            std::cout << "Received key X\n";
    const std::string& getName() const { return name; }
    int getGold() const { return gold; }
    const std::vector<Unit>& getUnits() const { return units; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Player{name=" << p.name
           << ", gold=" << p.gold
           << ", units=[";
        for (size_t i = 0; i < p.units.size(); ++i) {
            os << p.units[i];
            if (i + 1 < p.units.size()) os << ", ";
}
        os << "]}";
        return os;
    }
};

class Zone {
private:
    std::string name;
    std::vector<Unit> units;
public:
    Zone(const std::string& name = "")
        : name(name), units() {}

        if (IsKeyPressed(KEY_ESCAPE)) {
            shouldExit = true;
    Zone(const Zone& other)
        : name(other.name), units(other.units) {}

    Zone& operator=(const Zone& other) {
        if (this != &other) {
            name = other.name;
            units = other.units;
        }
        return *this;
    }

    ~Zone() = default;

    void addUnit(const Unit& u) {
        units.push_back(u);
    }

    int getZonePower() const {
        int total = 0;
        for (const auto& u : units) {
            total += u.getPower();
}
        return total;
    }

        if (shouldExit) {
            std::cout << "Fereastra a fost închisă (shouldExit == true)\n";
            break; // Ieșim din loop, fereastra se închide la distrugerea obiectului
    bool removeDeadUnits() {
        bool removed = false;
        std::vector<Unit> alive;
        alive.reserve(units.size());
        for (const auto& u : units) {
            if (u.isAlive()) {
                alive.push_back(u);
            } else {
                removed = true;
            }
}
        units.swap(alive);
        return removed;
    }

        // Simulare delay din exemplul tău (atenție: strică fluiditatea ferestrei)
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(300ms);
    const std::string& getName() const { return name; }
    const std::vector<Unit>& getUnits() const { return units; }

        // Randare
        BeginDrawing();
            window.ClearBackground(RAYWHITE); // Echivalentul lui window.clear()
            
            // Aici poți desena chestii
            
        EndDrawing(); // Echivalentul lui window.display()
    Unit getUnitAt(size_t index) const {
        return units.at(index);
}

    std::cout << "Fereastra a fost închisă\n";
    std::cout << "Programul a terminat execuția\n";
    void removeUnitAt(size_t index) {
        if (index < units.size()) {
            units.erase(units.begin() + static_cast<long>(index));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone{name=" << z.name
           << ", power=" << z.getZonePower()
           << ", units=[";
        for (size_t i = 0; i < z.units.size(); ++i) {
            os << z.units[i];
            if (i + 1 < z.units.size()) os << ", ";
        }
        os << "]}";
        return os;
    }
};

class Game {
private:
    std::vector<Player> players;
    std::vector<Zone> zones;
public:
    Game() = default;

    Game(const Game& other)
        : players(other.players), zones(other.zones) {}

    Game& operator=(const Game& other) {
        if (this != &other) {
            players = other.players;
            zones = other.zones;
        }
        return *this;
    }

    ~Game() = default;

    void addPlayer(const Player& p) {
        players.push_back(p);
    }

    void addZone(const Zone& z) {
        zones.push_back(z);
    }

    void moveUnit(Zone& from, size_t unitIndex, Zone& to) {
        if (unitIndex >= from.getUnits().size()) return;
        Unit u = from.getUnitAt(unitIndex);
        from.removeUnitAt(unitIndex);
        to.addUnit(u);
    }

    void battle(Zone& z) {
        auto& units = const_cast<std::vector<Unit>&>(z.getUnits());
        if (units.size() < 2) {
            std::cout << "Not enough units in zone " << z.getName() << " for battle.\n";
            return;
        }
        Unit& a = units[0];
        Unit& b = units[1];

        std::cout << "Battle in zone " << z.getName() << " between "
                  << a.getName() << " and " << b.getName() << "\n";

        while (a.isAlive() && b.isAlive()) {
            b.takeDamage(a.getAttack());
            if (!b.isAlive()) break;
            a.takeDamage(b.getAttack());
        }

        z.removeDeadUnits();
    }

    const std::vector<Player>& getPlayers() const { return players; }
    const std::vector<Zone>& getZones() const { return zones; }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "Game{\n  Players:\n";
        for (const auto& p : g.players) {
            os << "    " << p << "\n";
        }
        os << "  Zones:\n";
        for (const auto& z : g.zones) {
            os << "    " << z << "\n";
        }
        os << "}";
        return os;
    }
};

int main() {
    Unit swordsman("Swordsman", 100, 20);
    Unit archer("Archer", 70, 25);
    Unit knight("Knight", 120, 30);

    Player p1("Player1", 100);
    Player p2("Player2", 80);

    p1.addUnit(swordsman);
    p1.addUnit(archer);
    p2.addUnit(knight);

    Zone forest("Forest");
    Zone hill("Hill");

    forest.addUnit(swordsman);
    forest.addUnit(knight);
    hill.addUnit(archer);

    Game game;
    game.addPlayer(p1);
    game.addPlayer(p2);
    game.addZone(forest);
    game.addZone(hill);

    std::cout << "Initial game state:\n" << game << "\n\n";

    game.battle(const_cast<Zone&>(game.getZones()[0]));

    std::cout << "\nAfter battle in Forest:\n" << game << "\n\n";

return 0;
}
