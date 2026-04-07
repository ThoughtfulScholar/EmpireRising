#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <memory>
#include <raylib.h>

// ==========================================
// 1. CLASA ITEM (Compunere pentru Unit)
// ==========================================
class Item {
private:
    std::string name;
    int atkBonus;
    int defBonus;
    int price;
public:
    explicit Item(std::string n = "None", int a = 0, int d = 0, int p = 0)
        : name(std::move(n)), atkBonus(a), defBonus(d), price(p) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        return os << i.name << "(+" << i.atkBonus << "ATK/+" << i.defBonus << "DEF)";
    }
};

// ==========================================
// 2. CLASA ABILITY (Compunere pentru Unit)
// ==========================================
class Ability {
private:
    std::string name;
    float procChance;
    int power;
public:
    explicit Ability(std::string n = "None", float c = 0.0f, int p = 0)
        : name(std::move(n)), procChance(c), power(p) {}

    [[nodiscard]] int trigger() const {
        if (((float)rand() / RAND_MAX) < procChance) return power;
        return 0;
    }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        return os << a.name << "(" << (int)(a.procChance * 100) << "%)";
    }
};

// ==========================================
// 3. CLASA UNIT (Regula celor 3)
// ==========================================
enum class UnitType { INFANTRY, ARCHER, CAVALRY };

class Unit {
private:
    std::string name;
    UnitType type;
    int health;
    int maxHealth;
    int attack;
    int defense;
    Ability specialAbility; // Compunere 1
    Item gear;              // Compunere 2
    Color color;

public:
    Unit(std::string n, UnitType t, int h, int a, int d, Ability ab, Color c)
        : name(std::move(n)), type(t), health(h), maxHealth(h), attack(a), defense(d), specialAbility(std::move(ab)), gear(), color(c) {}

    // REGULA CELOR TREI (Cerință Tema 1)
    Unit(const Unit& other) 
        : name(other.name), type(other.type), health(other.health), maxHealth(other.maxHealth), 
          attack(other.attack), defense(other.defense), specialAbility(other.specialAbility), gear(other.gear), color(other.color) {}

    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            type = other.type;
            health = other.health;
            maxHealth = other.maxHealth;
            attack = other.attack;
            defense = other.defense;
            specialAbility = other.specialAbility;
            gear = other.gear;
            color = other.color;
        }
        return *this;
    }
    ~Unit() = default;

    // Funcție Netrivială 1: Calcul Damage cu Tipuri și Iteme
    [[nodiscard]] int calculateOutput(UnitType enemyType) const {
        float multiplier = 1.0f;
        if (type == UnitType::INFANTRY && enemyType == UnitType::ARCHER) multiplier = 1.5f;
        if (type == UnitType::ARCHER && enemyType == UnitType::CAVALRY) multiplier = 1.5f;
        if (type == UnitType::CAVALRY && enemyType == UnitType::INFANTRY) multiplier = 1.5f;

        int totalAtk = attack + gear.getAtk();
        int bonus = specialAbility.trigger();
        return static_cast<int>(totalAtk * multiplier) + bonus;
    }

    void takeDamage(int dmg) {
        int finalDmg = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - finalDmg);
    }

    void equip(const Item& it) { gear = it; }
    [[nodiscard]] bool isAlive() const { return health > 0; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] Color getColor() const { return color; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << u.name << " [" << u.health << "/" << u.maxHealth << "] Gear: " << u.gear;
        return os;
    }
};

// ==========================================
// 4. CLASA TERRAIN (Compunere pentru Zone)
// ==========================================
class Terrain {
private:
    std::string type;
    float bonus;
    Color tint;
public:
    explicit Terrain(std::string t = "Plains", float b = 1.0f, Color c = GREEN)
        : type(std::move(t)), bonus(b), tint(c) {}
    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] Color getTint() const { return tint; }
    [[nodiscard]] const std::string& getType() const { return type; }

    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) {
        return os << "Terrain: " << t.type;
    }
};

// ==========================================
// 5. CLASA QUEST (Obiective)
// ==========================================
class Quest {
private:
    std::string description;
    int reward;
    bool completed;
public:
    Quest(std::string d, int r) : description(std::move(d)), reward(r), completed(false) {}
    void complete() { completed = true; }
    [[nodiscard]] bool isDone() const { return completed; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string& getDesc() const { return description; }

    friend std::ostream& operator<<(std::ostream& os, const Quest& q) {
        return os << "Quest: " << q.description << " (" << (q.completed ? "DONE" : "OPEN") << ")";
    }
};

// ==========================================
// 6. CLASA ZONE (Gestiune Armate)
// ==========================================
class Zone {
private:
    std::string name;
    Terrain env; // Compunere 3
    std::vector<Unit> garrison;
    std::vector<std::string> battleHistory;
public:
    Zone(std::string n, Terrain t) : name(std::move(n)), env(t) {}

    void addUnit(const Unit& u) { garrison.push_back(u); }

    // Funcție Netrivială 2: Algoritm de luptă pe ture
    void runBattleRound() {
        if (garrison.size() < 2) return;
        
        int d1 = garrison[0].calculateOutput(garrison[1].getType());
        garrison[1].takeDamage(d1);
        battleHistory.push_back(garrison[0].getName() + " dealt " + std::to_string(d1));

        if (garrison[1].isAlive()) {
            int d2 = garrison[1].calculateOutput(garrison[0].getType());
            garrison[0].takeDamage(d2);
            battleHistory.push_back(garrison[1].getName() + " dealt " + std::to_string(d2));
        }

        garrison.erase(std::remove_if(garrison.begin(), garrison.end(), 
            [](const Unit& u) { return !u.isAlive(); }), garrison.end());
    }

    [[nodiscard]] const std::vector<Unit>& getUnits() const { return garrison; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const Terrain& getTerrain() const { return env; }
    [[nodiscard]] const std::vector<std::string>& getLogs() const { return battleHistory; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone: " << z.name << " (" << z.garrison.size() << " units)";
        return os;
    }
};

// ==========================================
// 7. CLASA PLAYER (Economie)
// ==========================================
class Player {
private:
    std::string name;
    int gold;
    std::vector<Item> inventory;
public:
    Player(std::string n, int g) : name(std::move(n)), gold(g) {}
    void earn(int amount) { gold += amount; }
    bool buy(int cost) {
        if (gold >= cost) { gold -= cost; return true; }
        return false;
    }
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << p.name << " Gold: " << p.gold;
    }
};

// ==========================================
// 8. CLASA GAME (Manager)
// ==========================================
class Game {
private:
    Player p;
    std::vector<Zone> world;
    std::vector<Quest> activeQuests;
    int frames;

public:
    explicit Game(Player player) : p(std::move(player)), frames(0) {}

    void init() {
        srand((unsigned)time(NULL));
        
        Zone forest("Deep Forest", Terrain("Forest", 0.8f, DARKGREEN));
        forest.addUnit(Unit("Knight", UnitType::INFANTRY, 150, 20, 10, Ability("Bash", 0.2f, 30), BLUE));
        forest.addUnit(Unit("Orc", UnitType::INFANTRY, 120, 25, 5, Ability("Rage", 0.1f, 50), RED));
        world.push_back(forest);

        Zone arena("Grand Arena", Terrain("Sand", 1.2f, YELLOW));
        arena.addUnit(Unit("Cavalry", UnitType::CAVALRY, 200, 30, 15, Ability("Charge", 0.3f, 40), BLUE));
        arena.addUnit(Unit("Spearman", UnitType::INFANTRY, 180, 15, 20, Ability("Pierce", 0.2f, 20), RED));
        world.push_back(arena);

        activeQuests.emplace_back("Defeat arena champion", 500);
    }

    // Funcție Netrivială 3: Sistemul de Randare și HUD
    void render() {
        ClearBackground(GetColor(0x101010FF));
        
        // Header
        DrawRectangle(0, 0, 800, 60, DARKGRAY);
        DrawText(TextFormat("COMMANDER: %s | GOLD: %i", p.getName().c_str(), p.getGold()), 20, 20, 20, GOLD);

        int x = 20;
        for (auto& z : world) {
            // Zone Box
            DrawRectangle(x, 80, 370, 500, Fade(z.getTerrain().getTint(), 0.3f));
            DrawRectangleLines(x, 80, 370, 500, GRAY);
            DrawText(z.getName().c_str(), x + 10, 90, 22, WHITE);
            DrawText(z.getTerrain().getType().c_str(), x + 10, 115, 16, LIGHTGRAY);

            // Units
            int y = 150;
            for (const auto& u : z.getUnits()) {
                DrawRectangle(x + 10, y, 350, 45, Fade(BLACK, 0.4f));
                float hpP = (float)u.getHP() / u.getMaxHP();
                DrawRectangle(x + 10, y + 40, (int)(350 * hpP), 5, u.getColor());
                DrawText(u.getName().c_str(), x + 20, y + 10, 18, WHITE);
                y += 55;
            }

            // Logs
            int ly = 450;
            auto logs = z.getLogs();
            int count = 0;
            for(auto it = logs.rbegin(); it != logs.rend() && count < 5; ++it, ++count) {
                DrawText(it->c_str(), x + 10, ly + (count * 15), 12, GRAY);
            }
            x += 390;
        }

        DrawText("SPACE: Fight | Q: Quests | M: Mine Gold", 200, 585, 16, RAYWHITE);
    }

    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& z : world) z.runBattleRound();
        }
        if (IsKeyPressed(KEY_M)) {
            p.earn(10);
        }
        if (IsKeyPressed(KEY_Q)) {
            std::cout << "--- ACTIVE QUESTS ---\n";
            for(const auto& q : activeQuests) std::cout << q << "\n";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        return os << "Game state: " << g.world.size() << " zones active.";
    }
};

int main() {
    InitWindow(800, 600, "Empire Rising v0.1");
    SetTargetFPS(60);

    Game engine(Player("Stefan", 100));
    engine.init();

    while (!WindowShouldClose()) {
        engine.handleInput();
        BeginDrawing();
        engine.render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
