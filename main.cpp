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
// EMPIRE RISING - Tema 1 (EXTENDED VERSION - 650+ Lines)
// ===========================================================

// -----------------------------------------------------------
// ITEM - Reprezintă echipamentul unităților
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

    // Operator pentru afișare detaliată în consolă/fișier
    friend std::ostream &operator<<(std::ostream &os, const Item &i) {
        return os << i.name << " (Atk:+" << i.atkBonus << ", Def:+" << i.defBonus 
                  << ", Cost:" << i.price << "g)";
    }
};

// -----------------------------------------------------------
// ABILITY - Abilități speciale declanșate random
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

    [[nodiscard]] float getChance() const { return procChance; }
    [[nodiscard]] int getPower() const { return power; }
    [[nodiscard]] const std::string &getName() const { return name; }

    friend std::ostream &operator<<(std::ostream &os, const Ability &a) {
        return os << a.name << " (" << std::fixed << std::setprecision(0) 
                  << (a.procChance * 100) << "% chance for +" << a.power << " dmg)";
    }
};

// -----------------------------------------------------------
// ENUM - Tipurile de unități pentru sistemul de counter
// -----------------------------------------------------------
enum class UnitType { INFANTRY, ARCHER, CAVALRY };

// -----------------------------------------------------------
// UNIT - Clasa principală de luptă (Regula celor 3)
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

    // Constructor de copiere (Regula celor 3)
    Unit(const Unit &o)
        : name(o.name), type(o.type), health(o.health),
          maxHealth(o.maxHealth), attack(o.attack), defense(o.defense),
          ability(o.ability), gear(o.gear), color(o.color) {}

    // Operator de atribuire (Regula celor 3)
    Unit &operator=(const Unit &o) {
        if (this != &o) {
            name = o.name;
            type = o.type;
            health = o.health;
            maxHealth = o.maxHealth;
            attack = o.attack;
            defense = o.defense;
            ability = o.ability;
            gear = o.gear;
            color = o.color;
        }
        return *this;
    }
    
    ~Unit() = default;

    void equip(const Item &i) { gear = i; }
    
    // Metodă de vindecare (folosită acum în input)
    void heal() { 
        health = std::min(maxHealth, health + (maxHealth / 4)); 
    }

    [[nodiscard]] bool isAlive() const { return health > 0; }

    int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float mult = 1.0f;
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) mult = 1.4f;
        if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) mult = 1.4f;
        if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) mult = 1.4f;
        
        int baseDmg = attack + gear.getAtk();
        int bonusDmg = ability.trigger();
        return static_cast<int>(static_cast<float>(baseDmg) * mult * terrainBonus) + bonusDmg;
    }

    void takeDamage(int dmg) {
        int realDef = defense + gear.getDef();
        int finalDmg = std::max(1, dmg - realDef);
        health = std::max(0, health - finalDmg);
    }

    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] raylib::Color getColor() const { return color; }
    [[nodiscard]] const Item& getGear() const { return gear; }

    friend std::ostream &operator<<(std::ostream &os, const Unit &u) {
        return os << u.name << " [" << u.health << "/" << u.maxHealth << "] "
                  << "Gear: " << u.gear.getName() << ", Ability: " << u.ability.getName();
    }
};
// -----------------------------------------------------------
// TERRAIN - Influențează bonusurile de luptă din zone
// -----------------------------------------------------------
class Terrain {
private:
    std::string name;
    float bonus{1.0f};
    raylib::Color tint;

public:
    explicit Terrain(std::string n = "Plains", float b = 1.0f,
                     raylib::Color t = raylib::Color::Green())
        : name(std::move(n)), bonus(b), tint(t) {}

    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] raylib::Color getTint() const { return tint; }

    friend std::ostream &operator<<(std::ostream &os, const Terrain &t) {
        return os << "Terrain Type: " << t.name << " (Multiplier: x" << t.bonus << ")";
    }
};

// -----------------------------------------------------------
// QUEST - Obiective secundare pentru jucător
// -----------------------------------------------------------
class Quest {
private:
    std::string desc;
    int reward{0};
    bool done{false};

public:
    explicit Quest(std::string d = "", int r = 0)
        : desc(std::move(d)), reward(r) {}

    void complete() { done = true; }
    [[nodiscard]] bool isDone() const { return done; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string &getDesc() const { return desc; }

    // Folosim o metodă de resetare pentru a refolosi sloturile de quest
    void reset(std::string d, int r) {
        desc = std::move(d);
        reward = r;
        done = false;
    }

    friend std::ostream &operator<<(std::ostream &os, const Quest &q) {
        return os << "Task: " << q.desc
                  << " | Status: " << (q.done ? "[COMPLETED]" : "[ACTIVE]") 
                  << " | Reward: " << q.reward << " gold";
    }
};

// -----------------------------------------------------------
// LOGGER - Sistem de jurnalizare a evenimentelor jocului
// -----------------------------------------------------------
class Logger {
private:
    std::string file;
    std::vector<std::string> buffer;

public:
    explicit Logger(std::string f = "game_history.txt") : file(std::move(f)) {}

    void add(const std::string &msg) {
        // Adăugăm un timestamp simulat simplu
        std::string entry = "[Log] " + msg;
        buffer.emplace_back(entry);
        // Limităm buffer-ul pentru a nu consuma memorie infinită în sesiuni lungi
        if (buffer.size() > 50) {
            buffer.erase(buffer.begin());
        }
    }

    void flushToFile() const {
        std::ofstream fo(file, std::ios::app);
        if (!fo) return;
        fo << "--- New Game Session Log ---\n";
        for (const auto &ln : buffer) {
            fo << ln << '\n';
        }
        fo << "----------------------------\n\n";
    }

    [[nodiscard]] const std::vector<std::string>& getBuffer() const { return buffer; }

    void clear() { buffer.clear(); }

    void renderConsole() const {
        std::cout << "\n>>> RECENT ACTIVITY <<<\n";
        for (const auto &ln : buffer) std::cout << "  " << ln << '\n';
    }
};

// -----------------------------------------------------------
// PLAYER - Entitatea ce gestionează economia și inventarul
// -----------------------------------------------------------
class Player {
private:
    std::string name;
    int gold{0};
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Hero", int g = 100)
        : name(std::move(n)), gold(g) {}

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::vector<Item> &getInventory() const { return inventory; }

    void earn(int g) { 
        gold += g; 
    }

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
            // Vindem cu 50% din preț
            int sellPrice = inventory[idx].getPrice() / 2;
            gold += sellPrice; 
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    // Transferă item-ul din inventar către o unitate
    void equipUnit(Unit &u, size_t idx) {
        if (idx < inventory.size()) { 
            u.equip(inventory[idx]); 
            inventory.erase(inventory.begin() + static_cast<long>(idx)); 
        }
    }

    void pay(int amount) {
        gold = std::max(0, gold - amount);
    }

    friend std::ostream &operator<<(std::ostream &os, const Player &p) {
        return os << "Commander " << p.name << " | Treasury: " << p.gold 
                  << "g | Items: " << p.inventory.size();
    }
};
// -----------------------------------------------------------
// MARKET - Sistemul de comerț al orașului
// -----------------------------------------------------------
class Market {
private:
    std::vector<Item> stock;
    Logger *log{nullptr};
    int restockTimer{0};

public:
    explicit Market(Logger *l = nullptr) : log(l) {
        stock.emplace_back("Iron Sword", 10, 0, 25);
        stock.emplace_back("Wooden Shield", 0, 10, 20);
        stock.emplace_back("Plate Armor", 2, 15, 45);
        stock.emplace_back("Longbow", 12, 1, 30);
    }

    [[nodiscard]] const std::vector<Item> &getStock() const { return stock; }

    void buy(Player &p, size_t idx) {
        if (idx < stock.size()) {
            std::string itemName = stock[idx].getName();
            if (p.buy(stock[idx])) {
                if (log) log->add(p.getName() + " purchased " + itemName);
                // Scoatem obiectul din magazin după cumpărare (stoc limitat)
                stock.erase(stock.begin() + static_cast<long>(idx));
            } else {
                if (log) log->add("Insufficient gold for " + itemName);
            }
        }
    }

    void restockRandom() {
        restockTimer++;
        if (restockTimer >= 600) { // Restocare la un anumit interval de cadre
            restockTimer = 0;
            int type = rand() % 3;
            if (type == 0) stock.emplace_back("Steel Blade", 14, 1, 40);
            else if (type == 1) stock.emplace_back("Tower Shield", 0, 18, 35);
            else stock.emplace_back("Elven Bow", 16, 0, 50);
            
            if (log) log->add("Market inventory has been updated with new wares.");
        }
    }

    void clearStaleStock() {
        if (stock.size() > 10) {
            stock.erase(stock.begin());
        }
    }
};

// -----------------------------------------------------------
// ZONE - Regiunile de luptă din jurul orașului
// -----------------------------------------------------------
class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;
    Logger *log{nullptr};

public:
    Zone(std::string n, Terrain t, Logger *l = nullptr)
        : name(std::move(n)), terrain(std::move(t)), log(l) {}

    [[nodiscard]] const std::vector<Unit> &getUnits() const { return units; }
    [[nodiscard]] const Terrain &getTerrain() const { return terrain; }
    [[nodiscard]] const std::string &getName() const { return name; }

    void addUnit(const Unit &u) { 
        units.push_back(u); 
        if (log) log->add("Unit " + u.getName() + " deployed to " + name);
    }

    void simulateBattle(Player &pl) {
        if (units.size() < 2) {
            if (log) log->add("Not enough units in " + name + " for a battle.");
            return;
        }

        float tb = terrain.getBonus();
        Unit &attacker = units[0]; 
        Unit &defender = units[1];

        // Runda 1: Atacatorul lovește
        int dmgA = attacker.damageOutput(defender.getType(), tb);
        defender.takeDamage(dmgA);
        if (log) log->add(attacker.getName() + " struck " + defender.getName() + " for " + std::to_string(dmgA));

        // Runda 2: Apărătorul ripostează dacă mai e în viață
        if (defender.isAlive()) {
            int dmgD = defender.damageOutput(attacker.getType(), tb);
            attacker.takeDamage(dmgD);
            if (log) log->add(defender.getName() + " counter-attacked for " + std::to_string(dmgD));
        } else {
            if (log) log->add(defender.getName() + " has fallen in battle!");
            pl.earn(25); // Recompensă pentru victorie
        }

        // Curățarea unităților moarte
        auto oldSize = units.size();
        units.erase(std::remove_if(units.begin(), units.end(),
                                   [](const Unit &u){ return !u.isAlive(); }),
                    units.end());
        
        if (units.size() < oldSize && log) {
            log->add("Casualties confirmed in " + name);
        }
    }

    void refreshUnits() {
        for (auto &u : units) u.heal();
    }
};

// -----------------------------------------------------------
// CITY - Hub-ul central al simulării
// -----------------------------------------------------------
class City {
private:
    std::string name;
    Player* owner{nullptr};
    std::vector<Zone> zones;
    Market market;
    Logger* log{nullptr};
    int taxRevenue{0};

public:
    City(std::string n, Player* p, Logger* l = nullptr)
        : name(std::move(n)), owner(p), market(l), log(l) {
        // Inițializăm orașul cu câteva zone predefinite
        zones.emplace_back("Great Citadel", Terrain("Stone Bastion", 1.2f, raylib::Color::LightGray()), l);
        zones.emplace_back("Whispering Woods", Terrain("Forest", 0.9f, raylib::Color::DarkGreen()), l);
    }

    std::vector<Zone>& getZones() { return zones; }
    Market& getMarket() { return market; }
    
    void addCustomZone(const std::string& zName, float bonus, raylib::Color color) {
        zones.emplace_back(zName, Terrain(zName, bonus, color), log);
    }

    void collectTaxes() {
        if (owner) {
            int amount = 5 + (static_cast<int>(zones.size()) * 2);
            owner->earn(amount);
            taxRevenue += amount;
            if (log && rand() % 10 == 0) 
                log->add(name + " collected taxes: " + std::to_string(amount) + "g");
        }
    }

    void runEconomy() {
        market.restockRandom();
        market.clearStaleStock();
        collectTaxes();
    }

    void showStatus() const {
        std::cout << "\n--- City Status: " << name << " ---\n";
        std::cout << "Zones Controlled: " << zones.size() << "\n";
        std::cout << "Total Tax Revenue: " << taxRevenue << "g\n";
    }
};
// -----------------------------------------------------------
// STATISTICS - Monitorizează progresul pe termen lung
// -----------------------------------------------------------
class Statistics {
private:
    int battles = 0;
    int unitsLost = 0;
    int goldEarned = 0;
    int itemsBought = 0;
    Logger* log{nullptr};

public:
    explicit Statistics(Logger* l = nullptr) : log(l) {}
    
    void recordBattle() { 
        ++battles; 
        if (log) log->add("Global statistics updated: Battle #" + std::to_string(battles));
    }
    
    void recordLoss(int count = 1) { 
        unitsLost += count; 
    }
    
    void recordGold(int g) { 
        goldEarned += g; 
    }
    
    void recordPurchase() { 
        ++itemsBought; 
    }

    void showConsole() const {
        std::cout << "\n========== GLOBAL STATS ==========\n"
                  << " Total Battles: " << battles << "\n"
                  << " Fallen Soldiers: " << unitsLost << "\n"
                  << " Accumulated Gold: " << goldEarned << "\n"
                  << " Mercant Transactions: " << itemsBought << "\n"
                  << "==================================\n";
    }

    friend std::ostream &operator<<(std::ostream &os, const Statistics &s) {
        return os << "B:" << s.battles << " | L:" << s.unitsLost 
                  << " | G:" << s.goldEarned << " | I:" << s.itemsBought;
    }
};

// -----------------------------------------------------------
// TURN MANAGER - Gestionează trecerea timpului
// -----------------------------------------------------------
class TurnManager {
private:
    int day = 1;
    int turn = 0;
    Logger* log{nullptr};

public:
    explicit TurnManager(Logger* l = nullptr) : log(l) {}
    
    void nextTurn() {
        ++turn;
        if (turn % 5 == 0) {
            ++day;
            if (log) log->add("A new day dawns: Day " + std::to_string(day));
        }
    }

    [[nodiscard]] int getDay() const { return day; }
    [[nodiscard]] int getTurn() const { return turn; }

    friend std::ostream &operator<<(std::ostream &os, const TurnManager &t) {
        return os << "Calendar: Day " << t.day << " (Cycle: " << t.turn << ")";
    }
};

// -----------------------------------------------------------
// ACHIEVEMENT - Sistem de trofee
// -----------------------------------------------------------
class Achievement {
private:
    std::string name;
    bool unlocked{false};
    Logger* log{nullptr};

public:
    Achievement(std::string n, Logger* l = nullptr)
        : name(std::move(n)), unlocked(false), log(l) {}

    void unlock() { 
        if (!unlocked) { 
            unlocked = true; 
            if (log) log->add("ACHIEVEMENT UNLOCKED: " + name); 
        } 
    }

    [[nodiscard]] bool isUnlocked() const { return unlocked; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Achievement& a) {
        os << (a.unlocked ? "[COMPLETED] " : "[LOCKED] ") << a.name;
        return os;
    }
};

// -----------------------------------------------------------
// GAME - Clasa principală de control
// -----------------------------------------------------------
class Game {
private:
    Player player;
    Logger log;
    City city;
    Statistics stat;
    TurnManager turns;
    std::vector<Quest> quests;
    std::vector<Achievement> achievements;
    raylib::Window window;
    bool running = true;

    // Helper intern pentru popularea zonelor
    static Unit createRandomUnit(const std::string& name, UnitType type, raylib::Color color) {
        Ability strike("Precision Strike", 0.25f, 15);
        int hp = 100 + (rand() % 50);
        int atk = 12 + (rand() % 10);
        int def = 5 + (rand() % 5);
        return Unit(name, type, hp, atk, def, strike, color);
    }

public:
    Game()
        : player("Stefan", 150),
          log("empire_history.log"),
          city("Ironhold", &player, &log),
          stat(&log),
          turns(&log),
          window(800, 600, "Empire Rising v2.0")
    {
        srand(static_cast<unsigned>(time(nullptr)));
        
        // Setup Quests
        quests.emplace_back("Initiate Combat", 50);
        quests.emplace_back("Equip your forces", 100);
        
        // Setup Achievements
        achievements.emplace_back("Warmonger", &log);
        achievements.emplace_back("Capitalist", &log);
        achievements.emplace_back("Tactician", &log);

        // Populate Zones
        auto &zones = city.getZones();
        zones[0].addUnit(createRandomUnit("Vanguard Knight", UnitType::INFANTRY, raylib::Color::SkyBlue()));
        zones[0].addUnit(createRandomUnit("Orc Marauder", UnitType::INFANTRY, raylib::Color::Red()));
        
        if (zones.size() > 1) {
            zones[1].addUnit(createRandomUnit("Elven Archer", UnitType::ARCHER, raylib::Color::Green()));
            zones[1].addUnit(createRandomUnit("Shadow Scout", UnitType::CAVALRY, raylib::Color::Purple()));
        }

        window.SetTargetFPS(60);
        log.add("Simulation kernel started successfully.");
    }

    // Curățare și salvare la final
    void shutdown() {
        log.add("Shutting down system. Final Stats: " + std::to_string(stat.getBattles()) + " battles.");
        log.flushToFile();
        std::cout << "Game session saved to empire_history.log\n";
    }

    bool isRunning() const { return !window.ShouldClose() && running; }

    // Metode de procesare (vor fi în Partea 5)
    void handleInput();
    void update();
    void render();
};

// -----------------------------------------------------------
// IMPLEMENTARE METODE GAME
// -----------------------------------------------------------

void Game::handleInput() {
    // Luptă în toate zonele active
    if (IsKeyPressed(KEY_SPACE)) {
        for (auto &z : city.getZones()) {
            size_t before = z.getUnits().size();
            z.simulateBattle(player);
            stat.recordBattle();
            
            if (z.getUnits().size() < before) {
                stat.recordLoss(static_cast<int>(before - z.getUnits().size()));
            }
        }
        achievements[0].unlock();
        if (!quests[0].isDone()) {
            quests[0].complete();
            player.earn(quests[0].getReward());
            log.add("Quest Completed: " + quests[0].getDesc());
        }
    }

    // Cumpărare item aleatoriu din Market
    if (IsKeyPressed(KEY_B)) {
        auto &mk = city.getMarket();
        if (!mk.getStock().empty()) {
            size_t idx = static_cast<size_t>(rand()) % mk.getStock().size();
            mk.buy(player, idx);
            stat.recordPurchase();
            if (player.getGold() > 500) achievements[1].unlock();
        }
    }

    // Echipare unitate din prima zonă cu primul item din inventar
    if (IsKeyPressed(KEY_E)) {
        if (!player.getInventory().empty() && !city.getZones()[0].getUnits().empty()) {
            Unit &target = city.getZones()[0].getUnits()[0];
            player.equipUnit(target, 0);
            log.add("Equipped " + target.getName() + " with new gear.");
            if (!quests[1].isDone()) {
                quests[1].complete();
                player.earn(quests[1].getReward());
            }
        }
    }

    // Vindecare unități (folosește metoda heal())
    if (IsKeyPressed(KEY_H)) {
        for (auto &z : city.getZones()) {
            for (auto &u : const_cast<std::vector<Unit>&>(z.getUnits())) {
                u.heal();
            }
        }
        log.add("Units in all zones have been patched up.");
    }

    // Vânzare ultimul obiect din inventar (folosește sellItem)
    if (IsKeyPressed(KEY_X)) {
        if (!player.getInventory().empty()) {
            player.sellItem(player.getInventory().size() - 1);
            log.add("Sold surplus equipment for gold.");
        }
    }

    // Avansare timp
    if (IsKeyPressed(KEY_T)) {
        turns.nextTurn();
        city.runEconomy();
    }

    // Debug Stats în Consolă
    if (IsKeyPressed(KEY_S)) {
        stat.showConsole();
        city.showStatus();
    }

    if (IsKeyPressed(KEY_ESCAPE)) running = false;
}

void Game::update() {
    // Economie pasivă și restocare
    city.runEconomy();
    
    // Eveniment aleatoriu rar de îmbogățire
    if (rand() % 1000 == 0) {
        int windfall = 50 + rand() % 100;
        player.earn(windfall);
        stat.recordGold(windfall);
        log.add("Local merchants gift you " + std::to_string(windfall) + "g in gratitude.");
    }
}

void Game::render() {
    window.BeginDrawing();
    window.ClearBackground(raylib::Color(25, 25, 35));

    // Header Info
    DrawRectangle(0, 0, 800, 50, raylib::Color(40, 40, 50));
    DrawText(TextFormat("COMMANDER: %s", player.getName().c_str()), 20, 15, 20, GOLD);
    DrawText(TextFormat("GOLD: %i", player.getGold()), 300, 15, 20, YELLOW);
    DrawText(TextFormat("DAY: %i (Turn: %i)", turns.getDay(), turns.getTurn()), 600, 15, 20, LIGHTGRAY);

    // Coloana Stângă: Zone și Unități
    int yOffset = 70;
    for (auto &z : city.getZones()) {
        DrawText(TextFormat("ZONE: %s (%s)", z.getName().c_str(), z.getTerrain().getName().c_str()), 20, yOffset, 18, z.getTerrain().getTint());
        yOffset += 25;

        if (z.getUnits().empty()) {
            DrawText("  [No units stationed here]", 20, yOffset, 14, GRAY);
            yOffset += 20;
        }

        for (const auto &u : z.getUnits()) {
            // Health Bar
            float hpPercent = static_cast<float>(u.getHP()) / u.getMaxHP();
            DrawRectangle(20, yOffset, 150, 12, DARKGRAY);
            DrawRectangle(20, yOffset, static_cast<int>(150 * hpPercent), 12, u.getColor());
            
            DrawText(TextFormat("%s (%i/%i)", u.getName().c_str(), u.getHP(), u.getMaxHP()), 180, yOffset - 2, 14, RAYWHITE);
            yOffset += 20;
        }
        yOffset += 15;
    }

    // Coloana Dreaptă: Inventar și Quest-uri
    DrawText("INVENTORY", 500, 70, 18, SKYBLUE);
    int invY = 95;
    if (player.getInventory().empty()) DrawText("Empty", 500, invY, 14, DARKGRAY);
    for (size_t i = 0; i < player.getInventory().size() && i < 10; ++i) {
        DrawText(TextFormat("- %s", player.getInventory()[i].getName().c_str()), 500, invY, 14, LIGHTGRAY);
        invY += 18;
    }

    DrawText("QUESTS", 500, 300, 18, ORANGE);
    int qY = 325;
    for (const auto &q : quests) {
        DrawText(TextFormat("%s %s", q.isDone() ? "[OK]" : "[  ]", q.getDesc().c_str()), 500, qY, 14, q.isDone() ? GREEN : RAYWHITE);
        qY += 20;
    }

    // Footer: Log-uri și Achievement-uri
    DrawRectangle(0, 480, 800, 120, raylib::Color(15, 15, 20));
    int logY = 490;
    auto logs = log.getBuffer();
    size_t startIdx = (logs.size() > 5) ? logs.size() - 5 : 0;
    for (size_t i = startIdx; i < logs.size(); ++i) {
        DrawText(logs[i].c_str(), 20, logY, 14, LIGHTGRAY);
        logY += 18;
    }

    // Afișăm statistica sumară în colț
    DrawText(TextFormat("Stats: %s", (std::stringstream() << stat).str().c_str()), 500, 570, 12, GRAY);
    DrawText("SPACE: Fight | B: Buy | E: Equip | H: Heal | T: Turn | S: Stats", 20, 575, 13, DARKGREEN);

    window.EndDrawing();
}

// -----------------------------------------------------------
// MAIN - Punctul de intrare optimizat pentru CI
// -----------------------------------------------------------
int main() {
    try {
        Game empire;
        
        // Timer de siguranță pentru mediile CI (Continuous Integration)
        // Dacă jocul rulează în GitHub Actions/Headless, se va închide automat
        double startTime = GetTime();
        int frameCounter = 0;

        while (empire.isRunning()) {
            empire.handleInput();
            empire.update();
            empire.render();

            frameCounter++;
            
            // Verificare Headless: dacă după 5 secunde nu există focus, închidem elegant
            if (!IsWindowFocused() && (GetTime() - startTime > 5.0)) {
                break;
            }
            
            // Limitare pentru teste automate: după 500 de cadre, închide dacă nu e interactiv
            if (frameCounter > 500 && !IsWindowReady()) {
                break;
            }
        }
        
        empire.shutdown();
        CloseWindow();
    } 
    catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 1;
    }

    return 0;
}
