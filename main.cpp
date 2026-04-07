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
#include <raylib-cpp.hpp>

// -----------------------------------------------------------
// ITEM
// -----------------------------------------------------------
class Item {
private:
    std::string name;
    int atkBonus{};
    int defBonus{};
    int price{};

public:
    explicit Item(std::string n = "None", int atk = 0, int def = 0, int p = 0)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(p) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string &getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Item &i) {
        return os << i.name << " (+" << i.atkBonus << "ATK/+" << i.defBonus << "DEF, "
                  << i.price << "g)";
    }
};

// -----------------------------------------------------------
// ABILITY
// -----------------------------------------------------------
class Ability {
private:
    std::string name;
    float procChance{};
    int power{};

public:
    explicit Ability(std::string n = "None", float c = 0.0f, int p = 0)
        : name(std::move(n)), procChance(c), power(p) {}

    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return (roll < procChance) ? power : 0;
    }

    friend std::ostream &operator<<(std::ostream &os, const Ability &a) {
        return os << a.name << " (" << int(a.procChance * 100) << "% +" << a.power << ")";
    }
};

// -----------------------------------------------------------
enum class UnitType { INFANTRY, ARCHER, CAVALRY };

// -----------------------------------------------------------
// UNIT
// -----------------------------------------------------------
class Unit {
private:
    std::string name;
    UnitType type;
    int health{};
    int maxHealth{};
    int attack{};
    int defense{};
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
    void heal() { health = maxHealth; }
    [[nodiscard]] bool isAlive() const { return health > 0; }

    int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float mult = 1.0f;
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) mult = 1.4f;
        if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) mult = 1.4f;
        if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) mult = 1.4f;
        return int((attack + gear.getAtk()) * mult * terrainBonus) + ability.trigger();
    }

    void takeDamage(int dmg) {
        int real = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - real);
    }

    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] int getHP() const { return health; }

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
    }

    void renderConsole() const {
        for (const auto &l : buffer)
            std::cout << l << '\n';
    }
};

// -----------------------------------------------------------
// PLAYER
// -----------------------------------------------------------
class Player {
private:
    std::string name;
    int gold{};
    std::vector<Item> inventory;

public:
    Player(std::string n = "Hero", int g = 100)
        : name(std::move(n)), gold(g) {}

    void earn(int g) { gold += g; }

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

    const std::vector<Item>& getInventory() const { return inventory; }
    int getGold() const { return gold; }
};

// -----------------------------------------------------------
// ZONE
// -----------------------------------------------------------
class Zone {
private:
    std::vector<Unit> units;

public:
    void addUnit(const Unit &u) { units.push_back(u); }

    std::vector<Unit>& getUnits() { return units; }

    void simulateBattle() {
        if (units.size() < 2) return;

        Unit &a = units[0];
        Unit &b = units[1];

        b.takeDamage(a.damageOutput(UnitType::INFANTRY));
        if (b.isAlive())
            a.takeDamage(b.damageOutput(UnitType::INFANTRY));

        units.erase(std::remove_if(units.begin(), units.end(),
                                   [](const Unit &u){ return !u.isAlive(); }),
                    units.end());
    }
};
// -----------------------------------------------------------
// EVENT
// -----------------------------------------------------------
class Event {
private:
    std::string name;
    int goldImpact{};
    Logger* log{};

public:
    Event(std::string n, int g, Logger* l = nullptr)
        : name(std::move(n)), goldImpact(g), log(l) {}

    void apply(Player& p) {
        p.earn(goldImpact);
        if (log) log->add("Event: " + name);
    }
};

// -----------------------------------------------------------
// STATISTICS
// -----------------------------------------------------------
class Statistics {
private:
    int battles = 0;

public:
    void recordBattle() { ++battles; }
    int getBattles() const { return battles; }
};

// -----------------------------------------------------------
// GAME
// -----------------------------------------------------------
class Game {
private:
    Player player;
    Logger log;
    Statistics stat;
    std::vector<Zone> zones;
    std::vector<Event> events;

public:
    Game() {
        srand((unsigned)time(nullptr));

        zones.resize(1);
        zones[0].addUnit(Unit("Knight", UnitType::INFANTRY, 100, 10, 5, Ability(), raylib::Color::Blue()));
        zones[0].addUnit(Unit("Orc", UnitType::INFANTRY, 100, 10, 5, Ability(), raylib::Color::Red()));

        events.emplace_back("Treasure", 20, &log);
        events.emplace_back("Raid", -10, &log);
    }

    Unit* findStrongestUnit() {
        Unit* best = nullptr;
        int maxHP = -1;

        for (auto &z : zones)
            for (auto &u : z.getUnits())
                if (u.getHP() > maxHP)
                    maxHP = u.getHP(), best = &u;

        return best;
    }
    void update() {
        if (rand() % 200 == 0) {
            events[rand() % events.size()].apply(player);
        }
    }

    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) {
            zones[0].simulateBattle();
            stat.recordBattle();
        }

        if (IsKeyPressed(KEY_E)) {
            if (!player.getInventory().empty()) {
                player.equip(zones[0].getUnits()[0], 0);
                log.add("Equipped item");
            }
        }

        if (IsKeyPressed(KEY_X)) {
            if (!player.getInventory().empty()) {
                player.sellItem(0);
                log.add("Sold item");
            }
        }

        if (IsKeyPressed(KEY_L)) {
            log.renderConsole();
        }

        if (IsKeyPressed(KEY_U)) {
            auto* u = findStrongestUnit();
            if (u) log.add("Strongest: " + u->getName());
        }
    }

    void run() {
        raylib::Window window(800, 600, "Game");

        while (!window.ShouldClose()) {
            handleInput();
            update();

            window.BeginDrawing();
            window.ClearBackground(raylib::Color::Black());
            raylib::DrawText("Empire Rising", 20, 20, 20, raylib::Color::White());
            window.EndDrawing();
        }
    }
};

// -----------------------------------------------------------
// MAIN
// -----------------------------------------------------------
int main() {
    Game g;
    g.run();
    return 0;
}
