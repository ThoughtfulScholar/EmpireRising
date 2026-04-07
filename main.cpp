#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <raylib.h>

// ==========================================
// 1. CLASA ABILITY (Compunere)
// ==========================================
class SpecialAbility {
private:
    std::string name;
    int damageBonus;
    float chance;
public:
    explicit SpecialAbility(std::string n = "None", int dmg = 0, float c = 0.0f)
        : name(std::move(n)), damageBonus(dmg), chance(c) {}

    [[nodiscard]] int getBonus() const {
        if (((float)rand() / RAND_MAX) < chance) return damageBonus;
        return 0;
    }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const SpecialAbility& a) {
        return os << a.name << " (Bonus:" << a.damageBonus << ")";
    }
};

// ==========================================
// 2. CLASA UNIT (Regula celor 3)
// ==========================================
class Unit {
private:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    int defense;
    SpecialAbility ability; // Compunere 1
    Color teamColor;

public:
    Unit(std::string n, int h, int a, int d, SpecialAbility sa, Color c)
        : name(std::move(n)), health(h), maxHealth(h), attack(a), defense(d), ability(std::move(sa)), teamColor(c) {}

    // REGULA CELOR TREI
    Unit(const Unit& other) 
        : name(other.name), health(other.health), maxHealth(other.maxHealth), 
          attack(other.attack), defense(other.defense), ability(other.ability), teamColor(other.teamColor) {}

    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            health = other.health;
            maxHealth = other.maxHealth;
            attack = other.attack;
            defense = other.defense;
            ability = other.ability;
            teamColor = other.teamColor;
        }
        return *this;
    }
    ~Unit() = default;

    // Funcție Netrivială 1: Calcul complex de luptă
    int calculateAttack(float terrainModifier) {
        int base = static_cast<int>(attack * terrainModifier);
        int bonus = ability.getBonus();
        if (bonus > 0) std::cout << name << " a activat " << ability.getName() << "!\n";
        return base + bonus;
    }

    void takeDamage(int dmg) {
        int finalDmg = std::max(1, dmg - defense);
        health = std::max(0, health - finalDmg);
    }

    [[nodiscard]] bool isAlive() const { return health > 0; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] Color getColor() const { return teamColor; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " [" << u.health << "/" << u.maxHealth << "] Ability: " << u.ability;
    }
};

// ==========================================
// 3. CLASA TERRAIN (Compunere pentru Zone)
// ==========================================
class Terrain {
private:
    std::string type;
    float atkMod;
    float defMod;
    Color tint;
public:
    explicit Terrain(std::string t = "Plains", float am = 1.0f, float dm = 1.0f, Color c = GREEN)
        : type(std::move(t)), atkMod(am), defMod(dm), tint(c) {}

    [[nodiscard]] float getAtkMod() const { return atkMod; }
    [[nodiscard]] const std::string& getType() const { return type; }
    [[nodiscard]] Color getTint() const { return tint; }

    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) {
        return os << "Terrain: " << t.type << " (Atk x" << t.atkMod << ")";
    }
};

// ==========================================
// 4. CLASA BUILDING (Mine de aur)
// ==========================================
class Building {
private:
    std::string bName;
    int goldPerTick;
public:
    explicit Building(std::string n = "Tent", int g = 0) : bName(std::move(n)), goldPerTick(g) {}
    [[nodiscard]] int collect() const { return goldPerTick; }
    [[nodiscard]] const std::string& getName() const { return bName; }

    friend std::ostream& operator<<(std::ostream& os, const Building& b) {
        return os << b.bName << " (+" << b.goldPerTick << "g)";
    }
};

// ==========================================
// 5. CLASA ZONE (Gestiune complexă)
// ==========================================
class Zone {
private:
    std::string name;
    Terrain env; // Compunere 2
    std::vector<Unit> army;
    Building structure; // Compunere 3
    std::vector<std::string> logs;

public:
    Zone(std::string n, Terrain t, Building b) : name(std::move(n)), env(t), structure(b) {}

    void addUnit(const Unit& u) { army.push_back(u); }

    // Funcție Netrivială 2: Simulare de luptă pe rânduri
    void processBattle() {
        if (army.size() < 2) return;
        
        int damageA = army[0].calculateAttack(env.getAtkMod());
        army[1].takeDamage(damageA);
        logs.push_back(army[0].getName() + " hits for " + std::to_string(damageA));

        if (army[1].isAlive()) {
            int damageB = army[1].calculateAttack(env.getAtkMod());
            army[0].takeDamage(damageB);
            logs.push_back(army[1].getName() + " hits for " + std::to_string(damageB));
        }

        // Curățare STL
        army.erase(std::remove_if(army.begin(), army.end(), 
            [](const Unit& u) { return !u.isAlive(); }), army.end());
    }

    int getIncome() const { return structure.collect(); }
    [[nodiscard]] const std::vector<Unit>& getUnits() const { return army; }
    [[nodiscard]] const Terrain& getTerrain() const { return env; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::vector<std::string>& getLogs() const { return logs; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone: " << z.name << " | " << z.env << " | Building: " << z.structure;
        return os;
    }
};

// ==========================================
// 6. CLASA PLAYER
// ==========================================
class Player {
private:
    std::string username;
    int gold;
public:
    Player(std::string n, int g) : username(std::move(n)), gold(g) {}
    void addGold(int amount) { gold += amount; }
    bool spend(int amount) {
        if (gold >= amount) { gold -= amount; return true; }
        return false;
    }
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return username; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Player: " << p.username << " | Gold: " << p.gold;
    }
};

// ==========================================
// 7. CLASA GAME (Managerul principal)
// ==========================================
class Game {
private:
    Player p;
    std::vector<Zone> world;
    int tickCounter;

public:
    explicit Game(Player player) : p(std::move(player)), tickCounter(0) {}

    void setup() {
        srand(static_cast<unsigned int>(time(NULL)));
        
        // Zona 1: Pădure cu Mină de aur mică
        Zone forest("Forbidden Woods", Terrain("Forest", 0.8f, 1.2f, DARKGREEN), Building("Small Mine", 10));
        forest.addUnit(Unit("Archer", 80, 25, 5, SpecialAbility("Double Shot", 20, 0.3f), BLUE));
        forest.addUnit(Unit("Spider", 120, 15, 8, SpecialAbility("Poison", 10, 0.5f), RED));
        world.push_back(forest);

        // Zona 2: Munte cu Fortăreață
        Zone mountain("Iron Peak", Terrain("Mountain", 1.5f, 0.7f, GRAY), Building("Gold Fortress", 50));
        mountain.addUnit(Unit("Knight", 200, 30, 15, SpecialAbility("Shield Bash", 15, 0.2f), BLUE));
        mountain.addUnit(Unit("Giant", 400, 40, 5, SpecialAbility("Stomp", 40, 0.1f), RED));
        world.push_back(mountain);
    }

    // Funcție Netrivială 3: Randare și Sistem de Tick-uri
    void render() {
        ClearBackground(GetColor(0x181818FF));
        
        DrawRectangle(0, 0, 800, 60, DARKGRAY);
        DrawText(TextFormat("PLAYER: %s | GOLD: %i", p.getName().c_str(), p.getGold()), 20, 20, 20, GOLD);

        int xPos = 20;
        for (auto& z : world) {
            // Desenare teren
            DrawRectangle(xPos, 80, 370, 500, z.getTerrain().getTint());
            DrawRectangleLines(xPos, 80, 370, 500, WHITE);
            DrawText(z.getName().c_str(), xPos + 10, 90, 22, BLACK);
            DrawText(TextFormat("Type: %s (Atk x%.1f)", z.getTerrain().getType().c_str(), z.getTerrain().getAtkMod()), xPos + 10, 115, 16, DARKGRAY);

            // Unități
            int yPos = 150;
            for (const auto& u : z.getUnits()) {
                DrawRectangle(xPos + 10, yPos, 350, 40, Fade(BLACK, 0.5f));
                float hpW = (float)u.getHP() / u.getMaxHP() * 100;
                DrawRectangle(xPos + 15, yPos + 30, (int)hpW, 5, u.getColor());
                DrawText(u.getName().c_str(), xPos + 20, yPos + 10, 18, WHITE);
                yPos += 50;
            }

            // Log-uri
            int logY = 450;
            auto logs = z.getLogs();
            int count = 0;
            for(auto it = logs.rbegin(); it != logs.rend() && count < 4; ++it, ++count) {
                DrawText(it->c_str(), xPos + 10, logY + (count * 15), 13, RAYWHITE);
            }
            xPos += 390;
        }

        DrawText("SPACE: Fight | G: Collect Gold", 250, 580, 18, LIGHTGRAY);
    }

    void handleEvents() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& z : world) z.processBattle();
        }
        if (IsKeyPressed(KEY_G)) {
            int total = 0;
            for (const auto& z : world) total += z.getIncome();
            p.addGold(total);
            std::cout << "Colected " << total << " gold from buildings.\n";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        return os << "Game state for " << g.p.getName() << " | Zones: " << g.world.size();
    }
};

// ==========================================
// 8. MAIN
// ==========================================
int main() {
    InitWindow(800, 600, "Empire Rising: Advanced OOP");
    SetTargetFPS(60);

    Game engine(Player("Stefan", 100));
    engine.setup();

    while (!WindowShouldClose()) {
        engine.handleEvents();
        BeginDrawing();
        engine.render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
