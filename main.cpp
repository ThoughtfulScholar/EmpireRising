#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <raylib-cpp.hpp>

// 1. CLASA ABILITY (Compunere)
class Ability {
private:
    std::string abilityName;
    int powerMultiplier;
public:
    // Constructor explicit pentru un singur parametru (Cerința Tema 1)
    explicit Ability(std::string name = "Normal", int mult = 1) 
        : abilityName(std::move(name)), powerMultiplier(mult) {}

    [[nodiscard]] int getMultiplier() const { return powerMultiplier; }
    [[nodiscard]] const std::string& getName() const { return abilityName; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        return os << "[" << a.abilityName << " x" << a.powerMultiplier << "]";
    }
};

// 2. CLASA UNIT (Regula celor 3)
class Unit {
private:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    Ability specialMove;
    raylib::Color unitColor;

public:
    Unit(std::string n, int h, int a, Ability ab, raylib::Color c)
        : name(std::move(n)), health(h), maxHealth(h), attack(a), specialMove(std::move(ab)), unitColor(c) {}

    // REGULA CELOR TREI
    Unit(const Unit& other) 
        : name(other.name), health(other.health), maxHealth(other.maxHealth), 
          attack(other.attack), specialMove(other.specialMove), unitColor(other.unitColor) {}

    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            health = other.health;
            maxHealth = other.maxHealth;
            attack = other.attack;
            specialMove = other.specialMove;
            unitColor = other.unitColor;
        }
        return *this;
    }
    ~Unit() = default;

    // Funcție Netrivială 1: Calcul complex atac
    [[nodiscard]] int calculateStrike() const {
        if (health < (maxHealth / 4)) return attack * 2; // Desperate strike
        return attack * specialMove.getMultiplier();
    }

    void applyDamage(int dmg) { health = std::max(0, health - dmg); }
    [[nodiscard]] bool isAlive() const { return health > 0; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] raylib::Color getColor() const { return unitColor; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " (HP: " << u.health << ") " << u.specialMove;
    }
};

// 3. CLASA ZONE (Compunere)
class Zone {
private:
    std::string zoneName;
    std::vector<Unit> garrison;
public:
    explicit Zone(std::string name) : zoneName(std::move(name)) {}

    void addUnit(const Unit& u) { garrison.push_back(u); }

    // Funcție Netrivială 2: Algoritm de luptă și curățare
    void resolveConflict() {
        if (garrison.size() < 2) return;
        garrison[1].applyDamage(garrison[0].calculateStrike());
        if (garrison[1].isAlive()) garrison[0].applyDamage(garrison[1].calculateStrike());

        garrison.erase(std::remove_if(garrison.begin(), garrison.end(), 
            [](const Unit& u) { return !u.isAlive(); }), garrison.end());
    }

    [[nodiscard]] const std::vector<Unit>& getGarrison() const { return garrison; }
    [[nodiscard]] const std::string& getName() const { return zoneName; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zona: " << z.zoneName << " | Unitati: " << z.garrison.size();
        for(const auto& u : z.garrison) os << "\n  -> " << u;
        return os;
    }
};

// 4. CLASA PLAYER
class Player {
private:
    std::string username;
    int gold;
public:
    Player(std::string n, int g) : username(std::move(n)), gold(g) {}
    [[nodiscard]] std::string getInfo() const { return username + " | Gold: " + std::to_string(gold); }
    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Player: " << p.username;
    }
};

// 5. CLASA GAME (Manager)
class Game {
private:
    Player player;
    std::vector<Zone> map;
public:
    explicit Game(Player p) : player(std::move(p)) {}

    void init() {
        Zone z("Arena");
        z.addUnit(Unit("Erou", 100, 20, Ability("Foc", 2), raylib::Color::Blue()));
        z.addUnit(Unit("Monstru", 150, 10, Ability(), raylib::Color::Red()));
        map.push_back(z);
    }

    // Funcție Netrivială 3: Randare Grafică
    void draw(raylib::Window& window) {
        window.ClearBackground(raylib::Color::Black());
        raylib::Text(player.getInfo(), 20, 20, 20, raylib::Color::Gold());
        int y = 80;
        for (const auto& z : map) {
            raylib::Text(z.getName(), 30, y, 22, raylib::Color::White());
            y += 30;
            for (const auto& u : z.getGarrison()) {
                raylib::DrawRectangle(40, y, (int)((float)u.getHP()/u.getMaxHP()*100), 15, u.getColor());
                y += 20;
            }
        }
    }

    void update() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& z : map) z.resolveConflict();
            std::cout << *this << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "Game status for " << g.player << "\nMap zones: " << g.map.size();
        return os;
    }
};

int main() {
    raylib::Window window(800, 600, "Empire Rising v0.1");
    SetTargetFPS(60);

    Game engine(Player("Stefan", 1000));
    engine.init();

    while (!window.ShouldClose()) {
        engine.update();
        BeginDrawing();
        engine.draw(window);
        EndDrawing();
    }
    return 0;
}
