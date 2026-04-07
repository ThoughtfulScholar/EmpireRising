#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <random>
#include <ctime>
#include <raylib-cpp.hpp>

// ===========================================================
// EMPIRE RISING v0.1  (TEMA 1 - OOP cu raylib-cpp)
// ===========================================================

// ===========================================================
// CLASS ITEM
// ===========================================================
class Item {
private:
    std::string name;
    int atkBonus{};
    int defBonus{};
    int price{};

public:
    Item(std::string n = "None", int atk=0, int def=0, int pr=0)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(pr) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Item& it) {
        os << it.name << " (+" << it.atkBonus << "ATK/+" << it.defBonus
           << "DEF, " << it.price << "g)";
        return os;
    }
};

// ===========================================================
// CLASS ABILITY
// ===========================================================
class Ability {
private:
    std::string name;
    float procChance;
    int power;

public:
    Ability(std::string n="None", float c=0.0f, int p=0)
        : name(std::move(n)), procChance(c), power(p) {}

    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(rand()) / RAND_MAX;
        return (roll < procChance) ? power : 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] float getChance() const { return procChance; }
    [[nodiscard]] int getPower() const { return power; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& ab) {
        os << ab.name << " (" << (int)(ab.procChance*100)
           << "% for +" << ab.power << ")";
        return os;
    }
};

// ===========================================================
// ENUMERARE - tipuri unități
// ===========================================================
enum class UnitType { INFANTRY, ARCHER, CAVALRY };

// ===========================================================
// CLASS UNIT  (Regula celor 3, compune Item + Ability)
// ===========================================================
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

    // Regula celor 3
    Unit(const Unit& o)
        : name(o.name), type(o.type), health(o.health), maxHealth(o.maxHealth),
          attack(o.attack), defense(o.defense), ability(o.ability), gear(o.gear), color(o.color) {}

    Unit& operator=(const Unit& o) {
        if (this != &o) {
            name = o.name; type = o.type; health = o.health; maxHealth = o.maxHealth;
            attack = o.attack; defense = o.defense; ability = o.ability; gear = o.gear; color = o.color;
        }
        return *this;
    }

    ~Unit() = default;

    // Funcții utile
    void equip(const Item& it) { gear = it; }
    void heal() { health = maxHealth; }

    [[nodiscard]] bool isAlive() const { return health > 0; }

    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] raylib::Color getColor() const { return color; }

    // Funcție netrivială: calcul damage
    [[nodiscard]] int damageOutput(UnitType enemyType) const {
        float mult = 1.0f;
        if (type == UnitType::INFANTRY && enemyType == UnitType::ARCHER) mult = 1.4f;
        if (type == UnitType::ARCHER && enemyType == UnitType::CAVALRY) mult = 1.4f;
        if (type == UnitType::CAVALRY && enemyType == UnitType::INFANTRY) mult = 1.4f;
        int dmg = (attack + gear.getAtk()) * mult + ability.trigger();
        return dmg;
    }

    void takeDamage(int dmg) {
        int real = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - real);
    }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << u.name << " [" << u.health << "/" << u.maxHealth << "] "
           << u.gear << " " << u.ability;
        return os;
    }
};

// ===========================================================
// CLASS TERRAIN - compus în Zone
// ===========================================================
class Terrain {
private:
    std::string name;
    float bonus;
    raylib::Color tint;

public:
    Terrain(std::string n="Plains", float b=1.0f, raylib::Color t=raylib::Color::Green())
        : name(std::move(n)), bonus(b), tint(t) {}

    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] raylib::Color getTint() const { return tint; }

    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) {
        os << "Terrain(" << t.name << ", x" << t.bonus << ")";
        return os;
    }
};

// ===========================================================
// CLASS QUEST - obiective logice
// ===========================================================
class Quest {
private:
    std::string desc;
    int reward;
    bool done{};

public:
    Quest(std::string d="", int r=0) : desc(std::move(d)), reward(r) {}
    void complete() { done = true; }
    [[nodiscard]] bool isDone() const { return done; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string& getDesc() const { return desc; }

    friend std::ostream& operator<<(std::ostream& os, const Quest& q) {
        os << "Quest: " << q.desc << " [" << (q.done ? "DONE" : "OPEN")
           << "] reward " << q.reward << "g";
        return os;
    }
};

// ===========================================================
// CLASS LOGGER - scrie în fișier log evenimente joc / zone
// ===========================================================
class Logger {
private:
    std::string filename;
    std::vector<std::string> buffer;

public:
    explicit Logger(std::string file="log.txt") : filename(std::move(file)) {}

    void add(const std::string& msg) {
        buffer.push_back(msg);
        if (buffer.size() > 50) buffer.erase(buffer.begin());
    }

    void flushToFile() const {
        std::ofstream fout(filename.c_str(), std::ios::app);
        if (fout) {
            for (const auto& s : buffer) fout << s << "\n";
        }
    }

    void renderConsole() const {
        std::cout << "--- LOG ---\n";
        for (auto& s : buffer) std::cout << s << "\n";
    }
};

// ===========================================================
// CLASS PLAYER
// ===========================================================
class Player {
private:
    std::string name;
    int gold;
    std::vector<Item> inventory;

public:
    Player(std::string n="Unknown", int g=100)
        : name(std::move(n)), gold(g) {}

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return name; }

    void earn(int amount) { gold += amount; }

    bool buy(const Item& it) {
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
            inventory.erase(inventory.begin() + (long long)idx);
        }
    }

    void equip(Unit& u, size_t idx) {
        if (idx < inventory.size()) {
            u.equip(inventory[idx]);
            inventory.erase(inventory.begin() + (long long)idx);
        }
    }

    const std::vector<Item>& getInventory() const { return inventory; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << p.name << " (" << p.gold << "g, Items: " << p.inventory.size() << ")";
        return os;
    }
};

// ===========================================================
// CLASS MARKET - implementare economie locală
// ===========================================================
class Market {
private:
    std::vector<Item> stock;
    Logger* log;

public:
    explicit Market(Logger* l=nullptr) : log(l) {
        stock.emplace_back("Sword", 8, 0, 20);
        stock.emplace_back("Shield", 0, 8, 18);
        stock.emplace_back("Armor", 5, 5, 28);
        stock.emplace_back("Bow", 10, 2, 25);
    }

    const std::vector<Item>& getStock() const { return stock; }

    void buy(Player& pl, size_t idx) {
        if (idx < stock.size()) {
            bool ok = pl.buy(stock[idx]);
            if (ok && log) log->add(pl.getName() + " bought " + stock[idx].getName());
        }
    }

    void restockRandom() {
        static int counter = 0;
        if (++counter % 400 == 0) {
            stock.push_back(Item("Mystic Sword", 15, 5, 40));
        }
    }
};

// ===========================================================
// CLASS ZONE - logică de luptă
// ===========================================================
class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;
    Logger* log;

public:
    Zone(std::string n, Terrain t, Logger* l=nullptr)
        : name(std::move(n)), terrain(std::move(t)), log(l) {}

    const std::vector<Unit>& getUnits() const { return units; }
    const Terrain& getTerrain() const { return terrain; }
    const std::string& getName() const { return name; }

    void addUnit(const Unit& u) { units.push_back(u); }

    // funcție netrivială: luptă pe ture
    void simulateBattle(Player& pl) {
        if (units.size() < 2) return;
        Unit& a = units[0];
        Unit& b = units[1];
        int dmgA = a.damageOutput(b.getType());
        b.takeDamage(dmgA);
        if (log) log->add(a.getName() + " hits " + b.getName() + " for " + std::to_string(dmgA));

        if (b.isAlive()) {
            int dmgB = b.damageOutput(a.getType());
            a.takeDamage(dmgB);
            if (log) log->add(b.getName() + " hits back " + a.getName() + " for " + std::to_string(dmgB));
        }

        units.erase(std::remove_if(units.begin(), units.end(),
                                   [&](const Unit& u){ return !u.isAlive(); }),
                    units.end());
        if (units.size() == 1 && log) log->add("Battle ended in " + name);
        if (units.size() == 1) pl.earn(10);
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone: " << z.name << " (" << z.units.size() << " units)";
        return os;
    }
};

// === SFÂRȘIT PARTEA 1 ===


// ===========================================================
// CLASS CITY - compune Zone și Market
// ===========================================================
class City {
private:
    std::string name;
    Player* owner{};
    Market market;
    std::vector<Zone> districts;
    Logger* log;

public:
    City(std::string n, Player* p, Logger* l=nullptr)
        : name(std::move(n)), owner(p), market(l), log(l) {
        // inițializare cu 2 zone
        districts.emplace_back("Citadel", Terrain("Stone Keep", 1.0f, raylib::Color::Gray()), l);
        districts.emplace_back("Outskirts", Terrain("Farmlands", 1.1f, raylib::Color::Green()), l);
    }

    const std::vector<Zone>& getZones() const { return districts; }
    Market& getMarket() { return market; }

    void addZone(const Zone& z) { districts.push_back(z); }

    // Funcție netrivială: întreținere economică
    void runEconomy() {
        market.restockRandom();
        if (owner) owner->earn(1);
    }

    void renderSummary() const {
        std::cout << "=== " << name << " Summary ===\n";
        std::cout << "Owner: " << *owner << "\n";
        for (const auto& z : districts) std::cout << "- " << z << "\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "City: " << c.name << " (" << c.districts.size() << " zones)";
        return os;
    }
};

// ===========================================================
// CLASS GAME - manager principal (grafica + logica)
// ===========================================================
class Game {
private:
    Player player;
    Logger log;
    City capital;
    std::vector<Quest> quests;
    raylib::Window window;
    raylib::Color bgColor;
    bool running;

    static Unit makeUnit(const std::string& n, UnitType t, raylib::Color c) {
        int hp = 120 + rand() % 60;
        int atk = 15 + rand() % 8;
        int def = 5 + rand() % 6;
        Ability ab("Skill", 0.2f, 20);
        return Unit(n, t, hp, atk, def, ab, c);
    }

public:
    Game() :
        player("Stefan", 100),
        log("empire_log.txt"),
        capital("Eaglefort", &player, &log),
        window(800, 600, "Empire Rising v0.1"),
        bgColor(30,30,30), running(true)
    {
        srand((unsigned) time(nullptr));
        quests.emplace_back("Win your first battle", 100);
        quests.emplace_back("Equip a unit in Citadel", 75);

        // populăm zone
        Zone& citadel = const_cast<Zone&>(capital.getZones()[0]);
        citadel.addUnit(makeUnit("Knight", UnitType::INFANTRY, raylib::Color::Blue()));
        citadel.addUnit(makeUnit("Orc", UnitType::INFANTRY, raylib::Color::Red()));

        Zone& farms = const_cast<Zone&>(capital.getZones()[1]);
        farms.addUnit(makeUnit("Archer", UnitType::ARCHER, raylib::Color::Green()));
        farms.addUnit(makeUnit("Raider", UnitType::CAVALRY, raylib::Color::Maroon()));

        window.SetTargetFPS(60);
        log.add("Game initialized.");
    }

    // Funcție netrivială: randare completă cu UI
    void render() {
        window.ClearBackground(bgColor);
        DrawRectangle(0, 0, 800, 60, raylib::Color::DarkGray());
        DrawText(TextFormat("COMMANDER: %s  |  GOLD: %i", player.getName().c_str(), player.getGold()), 20, 20, 20, RAYWHITE);

        int x = 20;
        for (const auto& zone : capital.getZones()) {
            DrawRectangle(x, 80, 360, 460, Fade(zone.getTerrain().getTint(), 0.35f));
            DrawRectangleLines(x, 80, 360, 460, raylib::Color::Gray());
            DrawText(zone.getName().c_str(), x + 10, 90, 20, RAYWHITE);
            DrawText(zone.getTerrain().getName().c_str(), x + 10, 115, 16, raylib::Color::LightGray());

            int y = 160;
            for (const auto& u : zone.getUnits()) {
                DrawRectangle(x + 10, y, 340, 40, Fade(raylib::Color::Black(), 0.6f));
                float ratio = (float) u.getHP() / u.getMaxHP();
                DrawRectangle(x + 10, y + 35, (int)(340 * ratio), 5, u.getColor());
                DrawText(u.getName().c_str(), x + 20, y + 10, 16, RAYWHITE);
                y += 50;
            }

            x += 380;
        }

        DrawText("SPACE: Battle | M: Mine Gold | B: Buy Random | E: Equip | L: Log | Q: Quests | ESC: Exit",
                 30, 570, 15, raylib::Color::LightGray());
        window.EndDrawing();
    }

    // gestionează inputul și logica
    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& z : const_cast<std::vector<Zone>&>(capital.getZones()))
                z.simulateBattle(player);
        }

        if (IsKeyPressed(KEY_M)) {
            player.earn(10);
            log.add("Mined +10 gold.");
        }

        if (IsKeyPressed(KEY_B)) {
            // cumpără un item random
            auto& mkt = capital.getMarket();
            size_t idx = rand() % mkt.getStock().size();
            mkt.buy(player, idx);
        }

        if (IsKeyPressed(KEY_E)) {
            auto& zones = const_cast<std::vector<Zone>&>(capital.getZones());
            if (!zones.empty() && !zones[0].getUnits().empty()) {
                if (!player.getInventory().empty()) {
                    player.equip(const_cast<Unit&>(zones[0].getUnits()[0]), 0);
                    quests[1].complete();
                    log.add(player.getName() + " equipped a unit.");
                }
            }
        }

        if (IsKeyPressed(KEY_L)) {
            log.renderConsole();
        }

        if (IsKeyPressed(KEY_Q)) {
            std::cout << "--- ACTIVE QUESTS ---\n";
            for (const auto& q : quests) {
                std::cout << q << "\n";
                if (!q.isDone()) std::cout << "(Incomplete)\n";
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            running = false;
            log.add("Exit triggered.");
        }
    }

    bool shouldClose() const { return window.ShouldClose() || !running; }

    void update() {
        capital.runEconomy();
    }

    void gameLoop() {
        while (!shouldClose()) {
            window.BeginDrawing();
            handleInput();
            update();
            render();
        }
        log.flushToFile();
    }
};

// ===========================================================
// MAIN
// ===========================================================
int main() {
    try {
        Game empire;
        empire.gameLoop();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
