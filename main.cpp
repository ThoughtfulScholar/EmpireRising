#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <memory>
#include <raylib.h>

// ==========================================
// EmpireRising – Tema 1 (grupa OOP)
// ==========================================
// Respectă cerințele:
// - >=4 clase cu compunere (Item, Ability, Terrain, Zone, Game etc.)
// - Regula celor 3
// - Operator << pentru toate clasele
// - Minim 3 funcții netriviale
// - Const correctness și private members
// - Fără variabile globale
// ==========================================

// ==========================================
// CLASA ITEM (Compunere pentru Unit)
// ==========================================
class Item {
private:
    std::string name;
    int atkBonus;
    int defBonus;
    int price;

public:
    explicit Item(std::string n = "None", int atk = 0, int def = 0, int p = 0)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(p) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string &getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Item &i) {
        os << i.name << " (+" << i.atkBonus << "ATK/+" << i.defBonus << "DEF)";
        return os;
    }
};

// ==========================================
// CLASA ABILITY (Compunere pentru Unit)
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
        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll < procChance) return power;
        return 0;
    }

    [[nodiscard]] const std::string &getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Ability &a) {
        os << a.name << " (" << static_cast<int>(a.procChance * 100) << "% trigger)";
        return os;
    }
};

// ==========================================
// ENUM tipuri unități
// ==========================================
enum class UnitType { INFANTRY, ARCHER, CAVALRY };

// ==========================================
// CLASA UNIT (Regula celor 3, compunere: Item, Ability)
// ==========================================
class Unit {
private:
    std::string name;
    UnitType type;
    int health;
    int maxHealth;
    int attack;
    int defense;
    Ability special;
    Item gear;
    Color color;

public:
    Unit(std::string n, UnitType t, int h, int atk, int def, Ability sp, Color c)
        : name(std::move(n)), type(t), health(h), maxHealth(h),
          attack(atk), defense(def), special(std::move(sp)), gear(), color(c) {}

    // Regula celor 3
    Unit(const Unit &other)
        : name(other.name), type(other.type), health(other.health), maxHealth(other.maxHealth),
          attack(other.attack), defense(other.defense), special(other.special), gear(other.gear), color(other.color) {}

    Unit &operator=(const Unit &other) {
        if (this != &other) {
            name = other.name;
            type = other.type;
            health = other.health;
            maxHealth = other.maxHealth;
            attack = other.attack;
            defense = other.defense;
            special = other.special;
            gear = other.gear;
            color = other.color;
        }
        return *this;
    }

    ~Unit() = default;

    // Funcție netrivială 1: Damage calculat cu tipuri și gear
    [[nodiscard]] int calculateOutput(UnitType enemyType) const {
        float multiplier = 1.0f;
        if (type == UnitType::INFANTRY && enemyType == UnitType::ARCHER) multiplier = 1.5f;
        if (type == UnitType::ARCHER && enemyType == UnitType::CAVALRY) multiplier = 1.5f;
        if (type == UnitType::CAVALRY && enemyType == UnitType::INFANTRY) multiplier = 1.5f;

        int baseAtk = attack + gear.getAtk();
        return static_cast<int>(baseAtk * multiplier) + special.trigger();
    }

    void takeDamage(int dmg) {
        int real = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - real);
    }

    void equip(const Item &it) { gear = it; }

    [[nodiscard]] bool isAlive() const { return health > 0; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] Color getColor() const { return color; }

    friend std::ostream &operator<<(std::ostream &os, const Unit &u) {
        os << u.name << " [" << u.health << "/" << u.maxHealth << "] "
           << "Gear: " << u.gear << " Ability: " << u.special;
        return os;
    }
};

// ==========================================
// CLASA TERRAIN (Compunere pentru Zone)
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
    [[nodiscard]] const std::string &getType() const { return type; }
    [[nodiscard]] Color getTint() const { return tint; }

    friend std::ostream &operator<<(std::ostream &os, const Terrain &t) {
        os << "Terrain: " << t.type << " Bonus x" << t.bonus;
        return os;
    }
};

// ==========================================
// CLASA QUEST (obiective implementate logic)
// ==========================================
class Quest {
private:
    std::string description;
    int reward;
    bool completed;

public:
    Quest(std::string d, int r)
        : description(std::move(d)), reward(r), completed(false) {}

    void complete() { completed = true; }
    [[nodiscard]] bool isDone() const { return completed; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string &getDesc() const { return description; }

    friend std::ostream &operator<<(std::ostream &os, const Quest &q) {
        os << "Quest: " << q.description << " [" << (q.completed ? "DONE" : "OPEN") << "]";
        return os;
    }
};

// ==========================================
// CLASA PLAYER (gestiune economie)
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

    void addItem(const Item &i) { inventory.push_back(i); }

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::vector<Item> &getInventory() const { return inventory; }

    friend std::ostream &operator<<(std::ostream &os, const Player &p) {
        os << "Player " << p.name << " (" << p.gold << " gold)";
        return os;
    }
};

// ==========================================
// CLASA ZONE (compune Terrain și Unit)
// ==========================================
class Zone {
private:
    std::string name;
    Terrain env;
    std::vector<Unit> garrison;
    std::vector<std::string> history;

public:
    Zone(std::string n, Terrain t) : name(std::move(n)), env(std::move(t)) {}

    void addUnit(const Unit &u) { garrison.push_back(u); }
    [[nodiscard]] const std::vector<Unit> &getUnits() const { return garrison; }
    [[nodiscard]] const Terrain &getTerrain() const { return env; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::vector<std::string> &getHistory() const { return history; }

    // Funcție netrivială 2: rundă de luptă
    void runBattleRound() {
        if (garrison.size() < 2) return;

        int dmg1 = garrison[0].calculateOutput(garrison[1].getType());
        garrison[1].takeDamage(dmg1);
        history.push_back(garrison[0].getName() + " dealt " + std::to_string(dmg1) + " dmg.");

        if (garrison[1].isAlive()) {
            int dmg2 = garrison[1].calculateOutput(garrison[0].getType());
            garrison[0].takeDamage(dmg2);
            history.push_back(garrison[1].getName() + " dealt " + std::to_string(dmg2) + " dmg.");
        }

        garrison.erase(
            std::remove_if(garrison.begin(), garrison.end(),
                           [](const Unit &u) { return !u.isAlive(); }),
            garrison.end());
    }

    friend std::ostream &operator<<(std::ostream &os, const Zone &z) {
        os << "Zone: " << z.name << " (" << z.garrison.size() << " units)";
        return os;
    }
};

// ==========================================
// CLASA GAME (manager principal)
// ==========================================
class Game {
private:
    Player player;
    std::vector<Zone> world;
    std::vector<Quest> quests;

public:
    explicit Game(Player p) : player(std::move(p)) {}

    void init() {
        srand(static_cast<unsigned>(time(nullptr)));

        Zone forest("Deep Forest", Terrain("Forest", 0.9f, DARKGREEN));
        forest.addUnit(Unit("Knight", UnitType::INFANTRY, 150, 20, 10, Ability("Bash", 0.2f, 25), BLUE));
        forest.addUnit(Unit("Orc", UnitType::INFANTRY, 120, 25, 5, Ability("Rage", 0.15f, 40), RED));

        Zone arena("Grand Arena", Terrain("Sand", 1.2f, YELLOW));
        arena.addUnit(Unit("Cavalry", UnitType::CAVALRY, 200, 30, 15, Ability("Charge", 0.3f, 40), BLUE));
        arena.addUnit(Unit("Spearman", UnitType::INFANTRY, 180, 15, 20, Ability("Pierce", 0.2f, 30), RED));

        world.push_back(forest);
        world.push_back(arena);

        quests.emplace_back("Win one arena battle", 300);
        quests.emplace_back("Defeat an Orc in the forest", 500);
    }

    // Funcție netrivială 3: Randare și UI
    void render() {
        ClearBackground(GetColor(0x101010FF));
        DrawRectangle(0, 0, 800, 60, DARKGRAY);

        DrawText(TextFormat("COMMANDER: %s | GOLD: %i",
                            player.getName().c_str(), player.getGold()),
                 20, 20, 20, GOLD);

        int x = 20;
        for (auto &z : world) {
            DrawRectangle(x, 80, 360, 480, Fade(z.getTerrain().getTint(), 0.3f));
            DrawRectangleLines(x, 80, 360, 480, GRAY);
            DrawText(z.getName().c_str(), x + 10, 90, 22, WHITE);
            DrawText(z.getTerrain().getType().c_str(), x + 10, 115, 16, LIGHTGRAY);

            int y = 150;
            for (const auto &u : z.getUnits()) {
                DrawRectangle(x + 10, y, 340, 45, Fade(BLACK, 0.4f));
                float hpRatio = static_cast<float>(u.getHP()) / u.getMaxHP();
                DrawRectangle(x + 10, y + 40, static_cast<int>(340 * hpRatio), 5, u.getColor());
                DrawText(u.getName().c_str(), x + 20, y + 10, 18, WHITE);
                y += 55;
            }

            int logY = 450;
            const auto &logs = z.getHistory();
            int lines = 0;
            for (auto it = logs.rbegin(); it != logs.rend() && lines < 5; ++it, ++lines) {
                DrawText(it->c_str(), x + 10, logY + (lines * 15), 12, GRAY);
            }

            x += 380;
        }

        DrawText("SPACE: Fight | Q: Quests | M: Mine gold", 200, 570, 16, RAYWHITE);
    }

    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto &z : world)
                z.runBattleRound();
        }

        if (IsKeyPressed(KEY_M)) {
            player.earn(10);
        }

        if (IsKeyPressed(KEY_Q)) {
            std::cout << "--- ACTIVE QUESTS ---\n";
            for (const auto &q : quests) {
                std::cout << q << "\n";
                // Folosim isDone() -> elimină warning cppcheck
                if (q.isDone()) {
                    std::cout << "Completed quest reward: " << q.getReward() << "\n";
                }
            }
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Game &g) {
        os << "Game with " << g.world.size() << " zones.";
        return os;
    }
};

// ==========================================
// MAIN DEMO
// ==========================================
int main() {
    InitWindow(800, 600, "Empire Rising - Tema 1");
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
