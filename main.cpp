#include <iostream>
#include <string>
#include <vector>
#include <raylib.h>

// --- 1. Clasa Unit (Compoziție de bază) ---
class Unit {
private:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    Color teamColor;

public:
    explicit Unit(const std::string& n = "Recruit", int hp = 100, int atk = 10, Color c = GRAY)
        : name(n), health(hp), maxHealth(hp), attack(atk), teamColor(c) {}

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    // Funcții membru publice (Cerința: Funcționalități netriviale)
    bool isAlive() const { return health > 0; }
    int getPower() const { return health + attack; }
    
    // Getters const
    const std::string& getName() const { return name; }
    int getAttack() const { return attack; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    Color getColor() const { return teamColor; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit{n:" << u.name << ", hp:" << u.health << ", atk:" << u.attack << "}";
        return os;
    }
};

// --- 2. Clasa Player (Implementează Rule of Three conform cerinței) ---
class Player {
private:
    std::string name;
    int gold;
    Color color;
    std::vector<Unit> reserveUnits; // Compoziție

public:
    explicit Player(const std::string& n = "", int g = 0, Color c = BLUE)
        : name(n), gold(g), color(c) {}

    // --- RULE OF THREE ---
    Player(const Player& other) 
        : name(other.name), gold(other.gold), color(other.color), reserveUnits(other.reserveUnits) {
        // std::cout << "[Debug] Player Copy Constructor called\n";
    }

    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            color = other.color;
            reserveUnits = other.reserveUnits;
        }
        return *this;
    }

    ~Player() {
        // Destructor definit conform cerintei
    }
    // ----------------------

    bool spendGold(int amount) {
        if (amount > gold) return false;
        gold -= amount;
        return true;
    }

    void addGold(int amount) { gold += amount; }
    int getGold() const { return gold; }
    Color getColor() const { return color; }
    const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Player{name:" << p.name << ", gold:" << p.gold << ", units:" << p.reserveUnits.size() << "}";
        return os;
    }
};

// --- 3. Clasa Zone (Gestionare spațială și unități) ---
class Zone {
private:
    std::string name;
    Rectangle area;
    std::vector<Unit> units; // Compoziție

public:
    explicit Zone(const std::string& n = "", float x = 0, float y = 0, float w = 180, float h = 180)
        : name(n), area({x, y, w, h}) {}

    void addUnit(const Unit& u) { units.push_back(u); }

    // Funcție netrivială: Curățarea unităților moarte din zonă
    bool purgeDead() {
        size_t initial = units.size();
        for (auto it = units.begin(); it != units.end(); ) {
            if (!it->isAlive()) it = units.erase(it);
            else ++it;
        }
        return units.size() != initial;
    }

    int getZonePower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    // Randare Raylib
    void draw() const {
        DrawRectangleRec(area, Fade(DARKGRAY, 0.3f));
        DrawRectangleLinesEx(area, 2, LIGHTGRAY);
        DrawText(name.c_str(), static_cast<int>(area.x) + 10, static_cast<int>(area.y) + 10, 20, WHITE);
        
        for (size_t i = 0; i < units.size(); ++i) {
            int posX = static_cast<int>(area.x) + 30 + (static_cast<int>(i) % 4) * 35;
            int posY = static_cast<int>(area.y) + 60 + (static_cast<int>(i) / 4) * 40;
            DrawCircle(posX, posY, 12, units[i].getColor());
            
            // Bara de HP
            float hpBarWidth = 24.0f;
            float currentHpWidth = hpBarWidth * (static_cast<float>(units[i].getHealth()) / units[i].getMaxHealth());
            DrawRectangle(posX - 12, posY + 15, static_cast<int>(hpBarWidth), 4, MAROON);
            DrawRectangle(posX - 12, posY + 15, static_cast<int>(currentHpWidth), 4, GREEN);
        }
    }

    bool checkClick(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, area);
    }

    std::vector<Unit>& getUnits() { return units; }
    const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone{name:" << z.name << ", units:" << z.units.size() << "}";
        return os;
    }
};

// --- 4. Clasa Game (Clasa principală, Compoziție Game -> Players & Zones) ---
class Game {
private:
    std::vector<Player> players;
    std::vector<Zone> zones;
    std::string statusLog;

public:
    Game() : statusLog("Welcome, Commander!") {
        InitWindow(850, 650, "EmpireRising - Medieval Strategy");
        SetTargetFPS(60);
    }

    ~Game() { CloseWindow(); }

    void init() {
        players.emplace_back("Human", 200, BLUE);
        players.emplace_back("AI Enemy", 500, RED);

        zones.emplace_back("West Fortress", 50, 150);
        zones.emplace_back("Central Valley", 335, 150);
        zones.emplace_back("East Outpost", 620, 150);
    }

    // Funcție complexă: Gestionarea luptei în zone
    void processBattles() {
        for (auto& z : zones) {
            auto& units = z.getUnits();
            if (units.size() >= 2) {
                // Dacă unitățile aparțin unor echipe diferite, se atacă
                for (size_t i = 0; i < units.size(); ++i) {
                    for (size_t j = i + 1; j < units.size(); ++j) {
                        if (units[i].getColor().r != units[j].getColor().r) {
                            units[j].takeDamage(units[i].getAttack() / 10); // Daune pe frame (simulat)
                            units[i].takeDamage(units[j].getAttack() / 10);
                        }
                    }
                }
                if (z.purgeDead()) {
                    statusLog = "Units fell in " + z.getName();
                }
            }
        }
    }

    void handleInput() {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            for (auto& z : zones) {
                if (z.checkClick(m)) {
                    if (players[0].spendGold(30)) {
                        z.addUnit(Unit("Knight", 120, 25, players[0].getColor()));
                        statusLog = "Recruited Knight in " + z.getName();
                    }
                }
            }
        }
        // AI simplu: Adaugă unități periodic în Outpostul de Est
        if (GetTime() > 0 && static_cast<int>(GetTime()) % 5 == 0 && GetTime() - static_cast<int>(GetTime()) < 0.02) {
             zones[2].addUnit(Unit("Enemy", 80, 15, players[1].getColor()));
        }
    }

    void render() const {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        // Header UI
        DrawRectangle(0, 0, 850, 50, DARKGRAY);
        DrawText(TextFormat("GOLD: %d", players[0].getGold()), 20, 15, 20, GOLD);
        DrawText(statusLog.c_str(), 250, 15, 20, LIGHTGRAY);

        for (const auto& z : zones) z.draw();

        DrawText("Instruction: CLICK on a zone to buy a unit (30 Gold)", 20, 610, 18, GRAY);
        
        EndDrawing();
    }

    void run() {
        while (!WindowShouldClose()) {
            handleInput();
            processBattles();
            render();
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "Game Status: " << g.statusLog;
        return os;
    }
};

int main() {
    // Scenariu de utilizare conform cerinței
    Game engine;
    engine.init();
    
    // Afișare stare inițială folosind operator<< pentru a demonstra funcționarea
    std::cout << engine << std::endl;

    engine.run();

    return 0;
}
