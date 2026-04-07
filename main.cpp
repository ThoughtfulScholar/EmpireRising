#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <sstream>
#include <random>
#include <ctime>
#include <iomanip>
#include <raylib-cpp.hpp>

// ===========================================================
// EMPIRE RISING - Tema 1 (Partea 1)
// ===========================================================

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
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int calculateResaleValue() const { return static_cast<int>(price * 0.6); }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        return os << i.name << " (+" << i.atkBonus << "A/+" << i.defBonus << "D)";
    }
};

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

    [[nodiscard]] float getChance() const { return procChance; }
    [[nodiscard]] int getPower() const { return power; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        return os << a.name << " [" << std::fixed << std::setprecision(0) << (a.procChance * 100) << "%]";
    }
};

enum class UnitType { INFANTRY, ARCHER, CAVALRY };

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
    Unit(std::string n, UnitType t, int hp, int atk, int def, Ability ab, raylib::Color c)
        : name(std::move(n)), type(t), health(hp), maxHealth(hp),
          attack(atk), defense(def), ability(std::move(ab)), gear(), color(c) {}

    Unit(const Unit& o) : name(o.name), type(o.type), health(o.health), maxHealth(o.maxHealth),
                          attack(o.attack), defense(o.defense), ability(o.ability), gear(o.gear), color(o.color) {}

    Unit& operator=(const Unit& o) {
        if (this != &o) {
            name = o.name; type = o.type; health = o.health; maxHealth = o.maxHealth;
            attack = o.attack; defense = o.defense; ability = o.ability; gear = o.gear; color = o.color;
        }
        return *this;
    }
    ~Unit() = default;

    void equip(const Item& i) { gear = i; }
    void heal() { health = maxHealth; }
    [[nodiscard]] bool isAlive() const { return health > 0; }

    [[nodiscard]] int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float mult = 1.0f;
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) mult = 1.4f;
        if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) mult = 1.4f;
        if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) mult = 1.4f;
        return static_cast<int>((static_cast<float>(attack + gear.getAtk()) * mult * terrainBonus) + static_cast<float>(ability.trigger()));
    }

    void takeDamage(int dmg) {
        int real = std::max(1, dmg - (defense + gear.getDef()));
        health = std::max(0, health - real);
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] raylib::Color getColor() const { return color; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] const Ability& getAbility() const { return ability; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " (HP:" << u.health << ") " << u.gear << " | " << u.ability;
    }
};

class Terrain {
private:
    std::string name;
    float bonus{1.0f};
    raylib::Color tint;
public:
    explicit Terrain(std::string n = "Plains", float b = 1.0f, raylib::Color t = raylib::Color::Green())
        : name(std::move(n)), bonus(b), tint(t) {}
    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] raylib::Color getTint() const { return tint; }
    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) { return os << t.name; }
};

class Quest {
private:
    std::string desc;
    int reward{0};
    bool done{false};
public:
    explicit Quest(std::string d = "", int r = 0) : desc(std::move(d)), reward(r) {}
    void complete() { done = true; }
    [[nodiscard]] bool isDone() const { return done; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string& getDesc() const { return desc; }
    friend std::ostream& operator<<(std::ostream& os, const Quest& q) {
        return os << q.desc << (q.done ? " [COMPLETED]" : " [ACTIVE]");
    }
};

class Logger {
private:
    std::string file;
    std::vector<std::string> buffer;
public:
    explicit Logger(std::string f = "game.log") : file(std::move(f)) {}
    void add(const std::string& m) { buffer.push_back(m); if(buffer.size() > 5) buffer.erase(buffer.begin()); }
    void flush() const { std::ofstream fo(file, std::ios::app); for(auto& l : buffer) fo << l << "\n"; }
    [[nodiscard]] const std::vector<std::string>& getBuffer() const { return buffer; }
};
// -----------------------------------------------------------
// PLAYER, MARKET & ZONE - Managementul Logicii
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
    [[nodiscard]] const std::vector<Item>& getInv() const { return inventory; }

    bool buy(const Item& it) {
        if (gold >= it.getPrice()) { gold -= it.getPrice(); inventory.push_back(it); return true; }
        return false;
    }
    void sellItem(size_t idx) {
        if (idx < inventory.size()) { 
            gold += inventory[idx].calculateResaleValue(); 
            inventory.erase(inventory.begin() + (long)idx); 
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const Player& p) { 
        return os << p.name << " (" << p.gold << "g)"; 
    }
};

class Market {
private:
    std::vector<Item> stock;
public:
    Market() {
        stock.emplace_back("Steel Blade", 12, 0, 45);
        stock.emplace_back("Heavy Shield", 0, 15, 40);
        stock.emplace_back("Leather Tunic", 2, 8, 25);
    }
    const std::vector<Item>& getStock() const { return stock; }
    void restock() { 
        if(rand() % 500 == 0) {
            stock.emplace_back("Relic of War", 25, 10, 120);
        }
    }
};

class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;
public:
    Zone(std::string n, Terrain t) : name(std::move(n)), terrain(std::move(t)) {}
    void addUnit(const Unit& u) { units.push_back(u); }
    std::vector<Unit>& getUnits() { return units; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const Terrain& getTerrain() const { return terrain; }

    void processBattle(Player& p, Logger& log) {
        if (units.size() < 2) return;
        // Atacator vs Apărător
        int damage = units[0].damageOutput(units[1].getType(), terrain.getBonus());
        units[1].takeDamage(damage);
        log.add(units[0].getName() + " hits " + units[1].getName() + " for " + std::to_string(damage));
        
        // Contraatac dacă apărătorul supraviețuiește
        if (units[1].isAlive()) {
            int counter = units[1].damageOutput(units[0].getType(), terrain.getBonus());
            units[0].takeDamage(counter);
            log.add(units[1].getName() + " counters for " + std::to_string(counter));
        }

        // Curățare unități moarte
        for (auto it = units.begin(); it != units.end(); ) {
            if (!it->isAlive()) {
                log.add(it->getName() + " was slain!");
                it = units.erase(it);
                p.earn(30);
            } else { ++it; }
        }
    }
};

// -----------------------------------------------------------
// GAME ENGINE - Integrare Finală
// -----------------------------------------------------------
class Game {
private:
    Player player;
    Logger log;
    std::vector<Zone> world;
    Market market;
    Quest currentQuest;
    raylib::Window window;

public:
    Game() : player("Commander", 200), log("empire_rising.log"), 
             currentQuest("Defeat the Invaders", 150), window(800, 600, "Empire Rising - Strategy") {
        
        // Configurare lume
        world.emplace_back("Iron Hills", Terrain("Highlands", 1.2f, raylib::Color::Gray()));
        
        // Creare unități folosind compunerea
        Ability rush("Frenzied Rush", 0.35f, 20);
        Unit hero("Vanguard", UnitType::CAVALRY, 120, 22, 12, rush, raylib::Color::SkyBlue());
        Unit foe("Brute", UnitType::INFANTRY, 150, 15, 8, Ability("Heavy Swing", 0.2f, 25), raylib::Color::Maroon());
        
        world[0].addUnit(hero);
        world[0].addUnit(foe);
        
        window.SetTargetFPS(60);
    }

    void update() {
        market.restock();
        if (IsKeyPressed(KEY_SPACE)) world[0].processBattle(player, log);
        if (IsKeyPressed(KEY_B)) {
            if (!market.getStock().empty() && player.buy(market.getStock()[0])) 
                log.add("Item purchased!");
        }
        if (IsKeyPressed(KEY_S)) { 
            if (!player.getInv().empty()) { player.sellItem(0); log.add("Item sold!"); }
        }
        if (IsKeyPressed(KEY_H)) { 
            for(auto& u : world[0].getUnits()) u.heal(); 
            log.add("Reinforcements healed!"); 
        }
        if (IsKeyPressed(KEY_Q) && world[0].getUnits().size() < 2) {
            if(!currentQuest.isDone()) { 
                currentQuest.complete(); 
                player.earn(currentQuest.getReward()); 
                log.add("Objective secured!"); 
            }
        }
    }

    void render() {
        window.BeginDrawing();
        window.ClearBackground(raylib::Color(25, 25, 30));

        // Panou Player
        DrawText(TextFormat("Gold: %d | Inventory: %u", player.getGold(), (unsigned)player.getInv().size()), 20, 20, 22, GOLD);
        DrawText(currentQuest.getDesc().c_str(), 20, 50, 18, currentQuest.isDone() ? LIME : WHITE);

        // Randare Hartă și Unități
        int yOffset = 130;
        for (auto& z : world) {
            DrawText(z.getName().c_str(), 20, yOffset, 24, z.getTerrain().getTint());
            yOffset += 40;
            for (auto& u : z.getUnits()) {
                // Bara de viață proporțională
                float healthPct = (float)u.getHP() / u.getMaxHP();
                DrawRectangle(20, yOffset, 200, 15, BLACK);
                DrawRectangle(20, yOffset, (int)(200 * healthPct), 15, u.getColor());
                
                DrawText(TextFormat("%s (%d/%d HP)", u.getName().c_str(), u.getHP(), u.getMaxHP()), 230, yOffset, 16, WHITE);
                DrawText(u.getAbility().getName().c_str(), 450, yOffset, 14, GRAY);
                yOffset += 30;
            }
        }

        // Log Consola vizuală
        DrawText("LOGS:", 500, 380, 18, DARKGRAY);
        int ly = 410;
        for (const auto& msg : log.getBuffer()) {
            DrawText(msg.c_str(), 500, ly, 15, LIGHTGRAY);
            ly += 22;
        }

        DrawText("SPACE: Attack | B: Buy | S: Sell | H: Heal Units | Q: Claim Quest", 20, 560, 16, DARKGRAY);
        window.EndDrawing();
    }

    void run() {
        double runStart = GetTime();
        while (!window.ShouldClose()) {
            update();
            render();
            // CI Check: închide după 3 secunde dacă nu este interactiv (ex: server de build)
            if (!IsWindowFocused() && (GetTime() - runStart > 3.0)) break;
        }
        log.flush();
    }
};

// -----------------------------------------------------------
// MAIN
// -----------------------------------------------------------
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    try {
        Game empire;
        empire.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
