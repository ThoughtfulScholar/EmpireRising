#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <sstream>
#include <random>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <raylib-cpp.hpp>

// ===========================================================
// EMPIRE RISING - Versiune Extinsă (CI-Safe & No-Warnings)
// ===========================================================

// -----------------------------------------------------------
// ITEM
// -----------------------------------------------------------
class Item {
private:
    std::string name;
    int atkBonus{0};
    int defBonus{0};
    int price{0};

public:
    explicit Item(std::string n = "None", int atk = 0, int def = 0, int p = 0)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(p) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string &getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Item &i) {
        return os << i.name << " (+" << i.atkBonus << "A/+" << i.defBonus << "D)";
    }
};

// -----------------------------------------------------------
// ABILITY
// -----------------------------------------------------------
class Ability {
private:
    std::string name;
    float procChance{0.0f};
    int power{0};

public:
    explicit Ability(std::string n = "None", float c = 0.0f, int p = 0)
        : name(std::move(n)), procChance(c), power(p) {}

    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return (roll < procChance) ? power : 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Ability &a) {
        return os << a.name << " [" << std::fixed << std::setprecision(0) 
                  << (a.procChance * 100) << "%]";
    }
};

enum class UnitType { INFANTRY, ARCHER, CAVALRY };

// -----------------------------------------------------------
// UNIT
// -----------------------------------------------------------
class Unit {
private:
    std::string name;
    UnitType type;
    int health{0};
    int maxHealth{0};
    int attack{0};
    int defense{0};
    Ability ability;
    Item gear;
    raylib::Color color;

public:
    Unit(std::string n, UnitType t, int hp, int atk, int def,
         Ability ab, raylib::Color c)
        : name(std::move(n)), type(t), health(hp), maxHealth(hp),
          attack(atk), defense(def), ability(std::move(ab)), gear(), color(c) {}

    Unit(const Unit &o) = default;
    Unit &operator=(const Unit &o) = default;
    ~Unit() = default;

    void equip(const Item &i) { gear = i; }
    void heal(int amt) { health = std::min(maxHealth, health + amt); }
    [[nodiscard]] bool isAlive() const { return health > 0; }

    [[nodiscard]] int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float mult = 1.0f;
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) mult = 1.4f;
        if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) mult = 1.4f;
        if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) mult = 1.4f;
        return static_cast<int>((static_cast<float>(attack + gear.getAtk()) * mult * terrainBonus)) + ability.trigger();
    }

    void takeDamage(int dmg) {
        int real = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - real);
    }

    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] raylib::Color getColor() const { return color; }
    [[nodiscard]] UnitType getType() const { return type; }

    friend std::ostream &operator<<(std::ostream &os, const Unit &u) {
        return os << u.name << " [" << u.health << "/" << u.maxHealth << "]";
    }
};

// -----------------------------------------------------------
// LOGGER
// -----------------------------------------------------------
class Logger {
private:
    std::vector<std::string> buffer;

public:
    void add(const std::string &msg) {
        buffer.push_back(msg);
        if (buffer.size() > 8) buffer.erase(buffer.begin());
    }

    [[nodiscard]] const std::vector<std::string>& getLogs() const { return buffer; }

    void renderConsole() const {
        std::cout << "--- LOG HISTORY ---\n";
        for (const auto &l : buffer) std::cout << l << '\n';
    }
};
// -----------------------------------------------------------
// PLAYER
// -----------------------------------------------------------
class Player {
private:
    std::string name;
    int gold{0};
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Hero", int g = 100) : name(std::move(n)), gold(g) {}

    void earn(int g) { gold += g; }
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::vector<Item>& getInventory() const { return inventory; }

    bool buy(const Item &it) {
        if (gold >= it.getPrice()) {
            gold -= it.getPrice();
            inventory.push_back(it);
            return true;
        }
        return false;
    }

    void sellItem(size_t idx) {
        if (idx < inventory.size()) {
            gold += inventory[idx].getPrice() / 2;
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    void equip(Unit &u, size_t idx) {
        if (idx < inventory.size()) {
            u.equip(inventory[idx]);
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }
};

// -----------------------------------------------------------
// ZONE & EVENT
// -----------------------------------------------------------
class Zone {
private:
    std::string name;
    std::vector<Unit> units;

public:
    explicit Zone(std::string n) : name(std::move(n)) {}
    void addUnit(const Unit &u) { units.push_back(u); }
    std::vector<Unit>& getUnits() { return units; }
    [[nodiscard]] const std::string& getName() const { return name; }

    void simulateBattle(Logger& log) {
        if (units.size() < 2) return;
        Unit &a = units[0];
        Unit &b = units[1];

        int dmgA = a.damageOutput(b.getType());
        b.takeDamage(dmgA);
        log.add(a.getName() + " hits for " + std::to_string(dmgA));

        if (b.isAlive()) {
            int dmgB = b.damageOutput(a.getType());
            a.takeDamage(dmgB);
            log.add(b.getName() + " counters for " + std::to_string(dmgB));
        }

        units.erase(std::remove_if(units.begin(), units.end(),
            [](const Unit &u){ return !u.isAlive(); }), units.end());
    }
};

class Event {
private:
    std::string name;
    int goldImpact{0};
    Logger* log{nullptr};

public:
    Event(std::string n, int g, Logger* l = nullptr)
        : name(std::move(n)), goldImpact(g), log(l) {}

    void apply(Player& p) {
        p.earn(goldImpact);
        if (log) log->add("Event: " + name + " (" + std::to_string(goldImpact) + "g)");
    }
};

// -----------------------------------------------------------
// STATISTICS & GAME ENGINE
// -----------------------------------------------------------
class Statistics {
private:
    int battles{0};
    int itemsHandled{0};
public:
    void recordBattle() { ++battles; }
    void recordItem() { ++itemsHandled; }
    [[nodiscard]] int getBattles() const { return battles; }
    [[nodiscard]] int getItems() const { return itemsHandled; }
};

class Game {
private:
    Player player;
    Logger log;
    Statistics stat;
    std::vector<Zone> zones;
    std::vector<Event> events;
    std::vector<Item> market;

public:
    Game() : player("Stefan", 150) {
        zones.emplace_back("Frontier");
        zones[0].addUnit(Unit("Knight", UnitType::INFANTRY, 120, 18, 10, Ability("Bash", 0.3f, 15), raylib::Color::SkyBlue()));
        zones[0].addUnit(Unit("Orc", UnitType::INFANTRY, 150, 15, 5, Ability("Fury", 0.2f, 20), raylib::Color::Red()));

        events.emplace_back("Gold Mine", 50, &log);
        events.emplace_back("Ambush", -30, &log);
        
        market.emplace_back("Steel Sword", 10, 0, 40);
        market.emplace_back("Iron Shield", 0, 12, 35);
    }

    Unit* findStrongestUnit() {
        Unit* best = nullptr;
        int maxHP = -1;
        for (auto &z : zones) {
            for (auto &u : z.getUnits()) {
                if (u.getHP() > maxHP) { maxHP = u.getHP(); best = &u; }
            }
        }
        return best;
    }

    void update() {
        if (rand() % 500 == 0) events[static_cast<size_t>(rand()) % events.size()].apply(player);
    }

    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) { zones[0].simulateBattle(log); stat.recordBattle(); }
        if (IsKeyPressed(KEY_B) && !market.empty()) {
            if (player.buy(market[0])) { log.add("Bought " + market[0].getName()); stat.recordItem(); }
        }
        if (IsKeyPressed(KEY_E) && !player.getInventory().empty() && !zones[0].getUnits().empty()) {
            player.equip(zones[0].getUnits()[0], 0);
            log.add("Equipped item to " + zones[0].getUnits()[0].getName());
        }
        if (IsKeyPressed(KEY_X) && !player.getInventory().empty()) {
            player.sellItem(0);
            log.add("Sold item for gold");
        }
        if (IsKeyPressed(KEY_U)) {
            Unit* u = findStrongestUnit();
            if (u) log.add("MVP: " + u->getName() + " (" + std::to_string(u->getHP()) + " HP)");
        }
        if (IsKeyPressed(KEY_L)) log.renderConsole();
        if (IsKeyPressed(KEY_H) && !zones[0].getUnits().empty()) {
            for(auto &u : zones[0].getUnits()) u.heal(20);
            log.add("Healed units");
        }
    }

    void render(raylib::Window& window) {
        window.BeginDrawing();
        window.ClearBackground(raylib::Color(20, 20, 25));
        
        DrawText("EMPIRE RISING", 20, 20, 25, GOLD);
        DrawText(TextFormat("Player: %s | Gold: %d", player.getName().c_str(), player.getGold()), 20, 60, 20, RAYWHITE);
        DrawText(TextFormat("Battles: %d | Items: %d", stat.getBattles(), stat.getItems()), 20, 90, 18, LIGHTGRAY);

        int y = 140;
        DrawText(TextFormat("Zone: %s", zones[0].getName().c_str()), 20, y, 20, SKYBLUE);
        y += 30;
        for (auto &u : zones[0].getUnits()) {
            float healthBar = (float)u.getHP() / u.getMaxHP();
            DrawRectangle(20, y, 200, 15, DARKGRAY);
            DrawRectangle(20, y, (int)(200 * healthBar), 15, u.getColor());
            DrawText(u.getName().c_str(), 230, y, 16, RAYWHITE);
            y += 25;
        }

        DrawText("RECENT LOGS:", 500, 350, 18, GRAY);
        int ly = 380;
        for (const auto& msg : log.getLogs()) {
            DrawText(msg.c_str(), 500, ly, 15, LIGHTGRAY);
            ly += 20;
        }

        DrawText("SPACE: Fight | B: Buy | E: Equip | X: Sell | H: Heal | U: Find Strongest", 20, 560, 16, DARKGRAY);
        window.EndDrawing();
    }

    void run() {
        raylib::Window window(800, 600, "Empire Rising - CI Optimized");
        window.SetTargetFPS(60);
        double startTime = GetTime();

        while (!window.ShouldClose()) {
            handleInput();
            update();
            render(window);

            // Headless/CI Safety check
            if (!IsWindowFocused() && (GetTime() - startTime > 5.0)) break;
        }
    }
};

int main() {
    try {
        srand(static_cast<unsigned>(time(nullptr)));
        Game g;
        g.run();
    } catch (...) {
        return 1;
    }
    return 0;
}
