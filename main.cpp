// main.cpp
// EmpireRising - Tema 1 cu demo Raylib (single-file, demo implicit)

#include <iostream>
#include <string>
#include <vector>

#include "raylib.h"

// -------------------- Classes --------------------

class Unit {
private:
    std::string name;
    int health;
    int attack;

public:
    explicit Unit(const std::string& name = "", int health = 0, int attack = 0)
        : name(name), health(health), attack(attack) {}

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool isAlive() const { return health > 0; }
    int getPower() const { return health + attack; }

    const std::string& getName() const { return name; }
    int getAttack() const { return attack; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit{name=" << u.name
           << ", health=" << u.health
           << ", attack=" << u.attack << "}";
        return os;
    }
};

class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> units;

public:
    explicit Player(const std::string& name = "", int gold = 0)
        : name(name), gold(gold), units() {}

    // Rule of Three (cerință)
    Player(const Player& other)
        : name(other.name), gold(other.gold), units(other.units) {}

    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            units = other.units;
        }
        return *this;
    }

    ~Player() {}

    void addUnit(const Unit& u) { units.push_back(u); }

    int getTotalPower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    bool spendGold(int amount) {
        if (amount > gold) return false;
        gold -= amount;
        return true;
    }

    void gainGold(int amount) { gold += amount; }

    const std::string& getName() const { return name; }
    int getGold() const { return gold; }
    const std::vector<Unit>& getUnits() const { return units; }
    std::vector<Unit>& getUnits() { return units; }

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
    explicit Zone(const std::string& name = "")
        : name(name), units() {}

    void addUnit(const Unit& u) { units.push_back(u); }

    int getZonePower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    bool removeDeadUnits() {
        size_t before = units.size();
        std::vector<Unit> alive;
        alive.reserve(units.size());
        for (const auto& u : units)
            if (u.isAlive()) alive.push_back(u);
        units.swap(alive);
        return units.size() != before;
    }

    const std::string& getName() const { return name; }
    const std::vector<Unit>& getUnits() const { return units; }
    std::vector<Unit>& getUnits() { return units; }

    Unit getUnitAt(size_t index) const { return units.at(index); }

    void removeUnitAt(size_t index) {
        if (index < units.size())
            units.erase(units.begin() + index);
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

    void addPlayer(const Player& p) { players.push_back(p); }
    void addZone(const Zone& z) { zones.push_back(z); }

    const std::vector<Zone>& getZones() const { return zones; }
    std::vector<Zone>& getZones() { return zones; }
    const std::vector<Player>& getPlayers() const { return players; }
    std::vector<Player>& getPlayers() { return players; }

    void moveUnit(Zone& from, size_t index, Zone& to) {
        if (players.empty()) return; // folosit legitim pentru a evita sugestia static
        if (index >= from.getUnits().size()) return;
        Unit u = from.getUnitAt(index);
        from.removeUnitAt(index);
        to.addUnit(u);
    }

    void battle(Zone& z) {
        if (players.empty()) return; // folosit legitim pentru a evita sugestia static

        auto& units = z.getUnits();
        if (units.size() < 2) {
            // nu e suficient pentru luptă
            return;
        }

        Unit& a = units[0];
        Unit& b = units[1];

        while (a.isAlive() && b.isAlive()) {
            b.takeDamage(a.getAttack());
            if (!b.isAlive()) break;
            a.takeDamage(b.getAttack());
        }

        z.removeDeadUnits();
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "EmpireRising Game{\n  Players:\n";
        for (const auto& p : g.players)
            os << "    " << p << "\n";
        os << "  Zones:\n";
        for (const auto& z : g.zones)
            os << "    " << z << "\n";
        os << "}";
        return os;
    }
};

// -------------------- Demo main (Raylib) --------------------

static const int WIN_W = 1000;
static const int WIN_H = 700;

int main() {
    InitWindow(WIN_W, WIN_H, "EmpireRising - Raylib Demo");
    SetTargetFPS(60);

    Game game;

    Player p1("Player1", 120);
    Player p2("Player2", 80);

    Zone forest("Forest");
    Zone hill("Hill");

    forest.addUnit(Unit("Swordsman", 100, 20));
    forest.addUnit(Unit("Knight", 120, 30));
    hill.addUnit(Unit("Archer", 70, 25));

    game.addPlayer(p1);
    game.addPlayer(p2);
    game.addZone(forest);
    game.addZone(hill);

    // UI / interaction state
    int selectedZone = -1;
    int selectedUnitIndex = -1;

    // simple cooldown for recruit to avoid accidental spam
    int recruitCooldown = 0;

    while (!WindowShouldClose()) {
        // Input handling
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            int y = 120;
            selectedZone = -1;
            selectedUnitIndex = -1;
            for (size_t zi = 0; zi < game.getZones().size(); ++zi) {
                if (m.x >= 60 && m.x <= 460 && m.y >= y && m.y <= y + 120) {
                    selectedZone = static_cast<int>(zi);
                    int ux = 90;
                    for (size_t ui = 0; ui < game.getZones()[zi].getUnits().size(); ++ui) {
                        float cx = static_cast<float>(ux);
                        float cy = static_cast<float>(y + 60);
                        float dx = m.x - cx;
                        float dy = m.y - cy;
                        if (dx*dx + dy*dy <= 18.0f*18.0f) {
                            selectedUnitIndex = static_cast<int>(ui);
                            break;
                        }
                        ux += 70;
                    }
                    break;
                }
                y += 180;
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && selectedZone != -1 && selectedUnitIndex != -1) {
            Vector2 m = GetMousePosition();
            int y = 120;
            for (size_t zi = 0; zi < game.getZones().size(); ++zi) {
                if (m.x >= 60 && m.x <= 460 && m.y >= y && m.y <= y + 120) {
                    if (static_cast<int>(zi) != selectedZone) {
                        game.moveUnit(game.getZones()[selectedZone], selectedUnitIndex, game.getZones()[zi]);
                        selectedUnitIndex = -1;
                    }
                    break;
                }
                y += 180;
            }
        }

        if (IsKeyPressed(KEY_B)) {
            if (!game.getZones().empty()) game.battle(game.getZones()[0]);
        }

        if (IsKeyPressed(KEY_R) && recruitCooldown == 0) {
            // recruit a new basic unit for player1 if enough gold
            if (!game.getPlayers().empty()) {
                Player& owner = game.getPlayers()[0];
                const int cost = 30;
                if (owner.spendGold(cost)) {
                    owner.addUnit(Unit("Militia", 60, 10));
                    recruitCooldown = 30; // frames
                }
            }
        }

        if (recruitCooldown > 0) --recruitCooldown;

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("EmpireRising Demo", 20, 20, 28, DARKGREEN);
        DrawText("Left click: select unit | Right click: move | B: battle (zone 0) | R: recruit (30 gold)", 20, 60, 14, DARKGRAY);

        // Draw players HUD
        int hudX = 520;
        int hudY = 120;
        for (size_t pi = 0; pi < game.getPlayers().size(); ++pi) {
            const Player& pl = game.getPlayers()[pi];
            DrawText(pl.getName().c_str(), hudX, hudY, 20, BLACK);
            DrawText(("Gold: " + std::to_string(pl.getGold())).c_str(), hudX, hudY + 26, 16, DARKBLUE);
            DrawText(("Power: " + std::to_string(pl.getTotalPower())).c_str(), hudX, hudY + 48, 16, DARKRED);
            hudY += 100;
        }

        // Draw zones and units
        int y = 120;
        for (size_t zi = 0; zi < game.getZones().size(); ++zi) {
            Color rectColor = LIGHTGRAY;
            if (static_cast<int>(zi) == selectedZone) rectColor = BEIGE;
            DrawRectangle(60, y, 400, 120, rectColor);
            DrawText(game.getZones()[zi].getName().c_str(), 70, y + 8, 20, BLACK);

            int ux = 90;
            for (size_t ui = 0; ui < game.getZones()[zi].getUnits().size(); ++ui) {
                Color c = RED;
                if (static_cast<int>(zi) == selectedZone && static_cast<int>(ui) == selectedUnitIndex) c = BLUE;
                DrawCircle(ux, y + 60, 18, c);
                DrawText(game.getZones()[zi].getUnits()[ui].getName().c_str(), ux - 24, y + 84, 10, BLACK);
                ux += 70;
            }

            // zone power
            DrawText(("Power: " + std::to_string(game.getZones()[zi].getZonePower())).c_str(), 320, y + 8, 14, DARKGRAY);

            y += 180;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
