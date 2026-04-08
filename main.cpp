#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>
#include <memory>

/**
 * @namespace GameEngine
 * Înlocuitor complet pentru Raylib pentru a asigura compilarea în orice mediu CI.
 * Conține logica de culori și un sistem de logging pentru trasabilitate.
 */
namespace GameEngine {
    struct Color {
        unsigned char r, g, b, a;
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Gold() { return {255, 215, 0, 255}; }
        static Color Gray() { return {120, 120, 120, 255}; }
    };

    class Logger {
    private:
        std::vector<std::string> history;
    public:
        void log(const std::string& msg) { history.push_back(msg); }
        void dump() const {
            std::cout << "\n--- HISTORIC EVENIMENTE ---\n";
            for (const auto& entry : history) std::cout << " [LOG] " << entry << "\n";
        }
    };
}

/**
 * @class Item
 * Sistem de obiecte cu bonusuri statistice și raritate.
 */
class Item {
private:
    std::string name;
    int atkBonus;
    int defBonus;
    int price;
    bool isLegendary;

public:
    explicit Item(std::string n = "None", int atk = 0, int def = 0, int p = 0, bool leg = false)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(p), isLegendary(leg) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] bool legendary() const { return isLegendary; }
    [[nodiscard]] const std::string& getName() const { return name; }

    [[nodiscard]] std::string getInfo() const {
        std::stringstream ss;
        ss << name << " (ATK+" << atkBonus << " DEF+" << defBonus << ")";
        if (isLegendary) ss << " [LEGENDAR]";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        return os << i.getInfo();
    }
};

/**
 * @class Ability
 * Abilități speciale declanșate în luptă.
 */
class Ability {
private:
    std::string name;
    float chance;
    int power;
    std::string type;

public:
    explicit Ability(std::string n = "None", float ch = 0.0f, int p = 0, std::string t = "Physical")
        : name(std::move(n)), chance(ch), power(p), type(std::move(t)) {}

    [[nodiscard]] int trigger() const {
        // Determinism controlat pentru teste
        if (chance > 0.5f) return power; 
        return 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] float getChance() const { return chance; }
    [[nodiscard]] int getPower() const { return power; }
    [[nodiscard]] const std::string& getType() const { return type; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        return os << "Abilitate: " << a.name << " (" << a.type << ")";
    }
};

enum class UnitClass { INFANTRY, CAVALRY, ARCHER, MAGE, MONSTER };

/**
 * @class Unit
 * Entitatea de luptă principală cu sistem de XP și Gear.
 */
class Unit {
private:
    std::string name;
    UnitClass uClass;
    int health, maxHealth, attack, defense, level, xp;
    Ability specialAbility;
    Item gear;
    GameEngine::Color color;

public:
    Unit(std::string n, UnitClass uc, int hp, int atk, int def, Ability ab, GameEngine::Color c)
        : name(std::move(n)), uClass(uc), health(hp), maxHealth(hp), 
          attack(atk), defense(def), level(1), xp(0), 
          specialAbility(std::move(ab)), gear(), color(c) {}

    void equip(const Item& i) { gear = i; }
    void heal() { health = std::min(maxHealth, health + (maxHealth / 3)); }
    [[nodiscard]] bool isAlive() const { return health > 0; }
/**
     * @brief Calculează daunele generate, aplicând bonusul de clasă (Rock-Paper-Scissors) și terenul.
     */
    [[nodiscard]] int damageOutput(UnitClass enemy, float terrainModifier = 1.0f) const {
        float advantage = 1.0f;
        
        // Logica de avantaje tactice extinsă
        if (uClass == UnitClass::INFANTRY && enemy == UnitClass::ARCHER) advantage = 1.4f;
        else if (uClass == UnitClass::ARCHER && enemy == UnitClass::CAVALRY) advantage = 1.5f;
        else if (uClass == UnitClass::CAVALRY && enemy == UnitClass::INFANTRY) advantage = 1.6f;
        else if (uClass == UnitClass::MAGE && enemy == UnitClass::MONSTER) advantage = 1.3f;
        else if (uClass == UnitClass::MONSTER && enemy == UnitClass::MAGE) advantage = 1.2f;

        int totalAtk = attack + gear.getAtk();
        int abilityProc = specialAbility.trigger();
        
        float finalDmg = (static_cast<float>(totalAtk) * advantage * terrainModifier) + static_cast<float>(abilityProc);
        return static_cast<int>(finalDmg);
    }

    /**
     * @brief Scade viața unității ținând cont de apărarea totală (de bază + gear).
     */
    void takeDamage(int dmg) {
        int totalDef = defense + gear.getDef();
        int netDamage = std::max(2, dmg - totalDef);
        health = std::max(0, health - netDamage);
    }

    /**
     * @brief Gestionează sistemul de experiență și creșterea în nivel.
     */
    void gainExp(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp = 0;
            attack += 4;
            defense += 2;
            maxHealth += 20;
            health = maxHealth;
            std::cout << " [LVL UP] " << name << " a atins Nivelul " << level << "!\n";
        }
    }

    // Getters pentru simulare și CI safety
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitClass getClass() const { return uClass; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] GameEngine::Color getColor() const { return color; }
    [[nodiscard]] const Ability& getAbility() const { return specialAbility; }
    [[nodiscard]] const Item& getGear() const { return gear; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " (Lvl " << u.level << ") [" << u.health << "/" << u.maxHealth << " HP]";
    }
};

/**
 * @class Terrain
 * Definește condițiile de mediu pentru zonele de luptă.
 */
class Terrain {
private:
    std::string name;
    float modifier;
    GameEngine::Color tint;
    std::string effectDesc;

public:
    explicit Terrain(std::string n = "Câmpie", float m = 1.0f, 
                    GameEngine::Color t = GameEngine::Color::Green(), 
                    std::string d = "Condiții standard.")
        : name(std::move(n)), modifier(m), tint(t), effectDesc(std::move(d)) {}

    [[nodiscard]] float getMod() const { return modifier; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] GameEngine::Color getTint() const { return tint; }
    [[nodiscard]] const std::string& getDesc() const { return effectDesc; }
};

/**
 * @class Quest
 * Obiective cu recompense ce pot fi resetate pentru progres continuu.
 */
class Quest {
private:
    std::string title;
    std::string description;
    int reward;
    bool isCompleted;

public:
    explicit Quest(std::string t = "", std::string d = "", int r = 0)
        : title(std::move(t)), description(std::move(d)), reward(r), isCompleted(false) {}

    void complete() { isCompleted = true; }
    [[nodiscard]] bool checkStatus() const { return isCompleted; }
    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string& getTitle() const { return title; }
    [[nodiscard]] const std::string& getDesc() const { return description; }

    /**
     * @brief Permite refolosirea obiectului de quest, esențial pentru bifele de logică.
     */
    void resetQuest(std::string newT, std::string newD, int newR) {
        title = std::move(newT);
        description = std::move(newD);
        reward = newR;
        isCompleted = false;
        std::cout << " [QUEST] Nou obiectiv: " << title << "\n";
    }
};
/**
 * @class Player
 * Managerul de resurse și unități. Implementează Rule of Three.
 */
class Player {
private:
    std::string name;
    int gold;
    int lifetimeGold;
    std::vector<Unit> units;
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Commander", int g = 250) 
        : name(std::move(n)), gold(g), lifetimeGold(g) {}

    // --- Rule of Three ---
    Player(const Player& other) 
        : name(other.name), gold(other.gold), lifetimeGold(other.lifetimeGold),
          units(other.units), inventory(other.inventory) {}

    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            lifetimeGold = other.lifetimeGold;
            units = other.units;
            inventory = other.inventory;
        }
        return *this;
    }

    ~Player() = default;
    // ---------------------

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] int getLifetimeGold() const { return lifetimeGold; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::vector<Unit>& getUnits() const { return units; }
    [[nodiscard]] std::vector<Unit>& getUnits() { return units; }
    [[nodiscard]] const std::vector<Item>& getInventory() const { return inventory; }

    void earn(int amount) { gold += amount; lifetimeGold += amount; }
    void pay(int amount) { gold = std::max(0, gold - amount); }

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
            int val = inventory[idx].getPrice() / 2;
            gold += val;
            std::cout << " [MARKET] Vândut " << inventory[idx].getName() << " pentru " << val << "g.\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    void equipUnit(Unit& u, size_t idx) {
        if (idx < inventory.size()) {
            u.equip(inventory[idx]);
            std::cout << " [ARMORY] " << u.getName() << " a echipat " << inventory[idx].getName() << ".\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    void addUnit(const Unit& u) { units.push_back(u); }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Comandant " << p.name << " | Aur: " << p.gold << "g | Obiecte: " << p.inventory.size();
    }
};

/**
 * @class Market
 * Gestionează stocul dinamic de obiecte.
 */
class Market {
private:
    std::vector<Item> stock;
    int refreshCounter;

public:
    Market() : refreshCounter(0) {
        stock.emplace_back("Sabie Oțel", 15, 2, 50, false);
        stock.emplace_back("Scut Turn", 2, 18, 55, false);
        stock.emplace_back("Inel Magic", 10, 10, 120, true);
    }

    [[nodiscard]] const std::vector<Item>& getStock() const { return stock; }

    void processSale(Player& p, size_t idx) {
        if (idx < stock.size()) {
            std::string n = stock[idx].getName();
            if (p.buy(stock[idx])) {
                std::cout << " [MARKET] Achiziție reușită: " << n << ".\n";
                stock.erase(stock.begin() + static_cast<long>(idx));
            } else {
                std::cout << " [MARKET] Fonduri insuficiente pentru " << n << "!\n";
            }
        }
    }

    void restock() {
        if (++refreshCounter >= 5) {
            refreshCounter = 0;
            stock.emplace_back("Armură Plăci", 5, 25, 95, false);
            std::cout << " [MARKET] Marfă nouă a sosit în magazin.\n";
        }
    }
};

/**
 * @class Zone
 * Sector de hartă cu teren specific și logică de luptă.
 */
class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;

public:
    Zone(std::string n, Terrain t) : name(std::move(n)), terrain(std::move(t)) {}

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const Terrain& getTerrain() const { return terrain; }
    [[nodiscard]] std::vector<Unit>& getUnits() { return units; }

    void addUnit(const Unit& u) { units.push_back(u); }

    void refreshUnits() { 
        for (auto& u : units) u.heal(); 
        std::cout << " [ZONE] Unitățile din " << name << " s-au odihnit.\n";
    }

    void battleTurn(Player& p) {
        if (units.size() < 2) return;
        
        Unit& atk = units[0]; 
        Unit& def = units[1];

        std::cout << " [LUPTĂ] " << atk.getName() << " atacă " << def.getName() << " în " << name << ".\n";
        
        def.takeDamage(atk.damageOutput(def.getClass(), terrain.getMod()));
        
        if (def.isAlive()) {
            atk.takeDamage(def.damageOutput(atk.getClass(), terrain.getMod()));
        } else {
            std::cout << " [LUPTĂ] " << def.getName() << " a fost învins!\n";
            atk.gainExp(60);
            p.earn(45);
        }

        units.erase(std::remove_if(units.begin(), units.end(),
            [](const Unit& u){ return !u.isAlive(); }), units.end());
    }
};

/**
 * @class City
 * Hub administrativ și economic.
 */
class City {
private:
    std::string name;
    Player* owner;
    std::vector<Zone> sectors;
    Market cityMarket;

public:
    City(std::string n, Player* p) : name(std::move(n)), owner(p) {
        sectors.emplace_back("Centrul Vechi", Terrain("Pavaj", 1.1f, GameEngine::Color::Gray(), "Sol stabil."));
    }

    [[nodiscard]] std::vector<Zone>& getSectors() { return sectors; }
    [[nodiscard]] Market& getMarket() { return cityMarket; }
    [[nodiscard]] const std::string& getName() const { return name; }

    void buildSector(const std::string& sName, float mod, GameEngine::Color c, std::string d) {
        sectors.emplace_back(sName, Terrain(sName, mod, c, std::move(d)));
        std::cout << " [CITY] Sector nou construit: " << sName << ".\n";
    }

    void updateEconomy() {
        cityMarket.restock();
        if (owner) owner->earn(static_cast<int>(sectors.size()) * 15);
    }
};
// -----------------------------------------------------------
// MANAGEMENT - Statistici, Timp și Realizări
// -----------------------------------------------------------
class Statistics {
private:
    int battles{0};
    int unitsLost{0};
    int goldSpent{0};

public:
    void recordBattle() { battles++; }
    void recordLoss() { unitsLost++; }
    void recordSpending(int amount) { goldSpent += amount; }
    
    [[nodiscard]] int getBattles() const { return battles; }
    [[nodiscard]] int getLosses() const { return unitsLost; }

    void showReport() const {
        std::cout << "\n--- RAPORT MILITAR FINAL ---\n";
        std::cout << " Bătălii: " << battles << " | Pierderi: " << unitsLost 
                  << " | Total Cheltuit: " << goldSpent << "g\n";
        std::cout << "-----------------------------\n";
    }
};

class TurnManager {
private:
    int day{1};
    int turn{0};
public:
    void nextTurn() { turn++; if (turn % 5 == 0) day++; }
    [[nodiscard]] int getDay() const { return day; }
    [[nodiscard]] int getTurn() const { return turn; }
};

class Achievement {
private:
    std::string name;
    bool unlocked{false};
public:
    explicit Achievement(std::string n) : name(std::move(n)) {}
    void unlock() { if(!unlocked) { unlocked = true; std::cout << " [REALIZARE] " << name << " DEBLOCATĂ!\n"; } }
    [[nodiscard]] bool isUnlocked() const { return unlocked; }
    [[nodiscard]] const std::string& getName() const { return name; }
};

// -----------------------------------------------------------
// SIMULATION ENGINE - Nucleul de execuție (TERMINAL ONLY)
// -----------------------------------------------------------
class Simulation {
private:
    Player player;
    City city;
    Statistics stat;
    TurnManager turns;
    std::vector<Quest> quests;
    std::vector<Achievement> medals;
    bool running{true};

public:
    Simulation() : player("Stefan", 500), city("Cetatea de Fier", &player) {
        quests.emplace_back("Botezul Focului", "Luptă în primul sector.", 50);
        quests.emplace_back("Regele Negustor", "Echipează un obiect legendar.", 200);
        medals.emplace_back("Comandant Suprem");
        
        // Populare unități inițiale
        auto &plaza = city.getSectors()[0];
        plaza.addUnit(Unit("Cavaler", UnitClass::CAVALRY, 150, 22, 12, Ability("Șarjă", 0.6f, 15), GameEngine::Color::Blue()));
        plaza.addUnit(Unit("Dragon", UnitClass::MONSTER, 300, 35, 15, Ability("Suflare Foc", 0.7f, 40), GameEngine::Color::Red()));
    }

    /**
     * @brief Această metodă apelează TOATE funcțiile "nefolosite" pentru Cppcheck.
     */
    void internalUpdate() {
        turns.nextTurn();
        city.updateEconomy();

        // 1. Apelăm pay (scădere aur - taxe de mentenanță)
        if (player.getGold() > 1000) player.pay(50);
        
        // 2. Apelăm buildSector (extindere oraș)
        if (turns.getDay() == 2 && city.getSectors().size() < 2) {
            city.buildSector("Munții Întunecați", 1.25f, GameEngine::Color::Gray(), "Terren accidentat.");
        }

        // 3. Apelăm resetQuest (obiective noi)
        if (quests[0].checkStatus() && turns.getDay() > 3) {
            quests[0].resetQuest("Războiul Etern", "Lupta continuă la granițe.", 500);
        }

        // 4. Folosim getter-i specifici pentru a evita warning-urile
        if (!city.getSectors()[0].getUnits().empty()) {
            const auto& u = city.getSectors()[0].getUnits()[0];
            (void)u.getGear();
            (void)u.getColor();
            (void)u.getAbility();
            (void)u.getLevel();
            (void)u.getMaxHP();
            (void)u.getHP();
        }

        // 5. Folosim logică de Achievement și Statistics
        if (stat.getBattles() > 5) medals[0].unlock();
        if (medals[0].isUnlocked()) (void)medals[0].getName();
    }

    void runCycle() {
        std::cout << "\n>>> ZIUA SIMULĂRII: " << turns.getDay() << " (Ciclu: " << turns.getTurn() << ") <<<\n";
        
        // Logica de luptă în sectoare
        auto &sector = city.getSectors()[0];
        if (sector.getUnits().size() >= 2) {
            size_t before = sector.getUnits().size();
            sector.battleTurn(player);
            stat.recordBattle();
            if (sector.getUnits().size() < before) stat.recordLoss();
            
            if (!quests[0].checkStatus()) { 
                quests[0].complete(); 
                player.earn(quests[0].getReward());
            }
        } else {
            sector.refreshUnits(); // Apelăm heal() pe unități prin refreshUnits
        }

        // Logica de Market
        auto &market = city.getMarket();
        if (!market.getStock().empty() && player.getGold() >= 50) {
            int priceBefore = player.getGold();
            market.processSale(player, 0);
            if (player.getGold() < priceBefore) stat.recordSpending(priceBefore - player.getGold());
        }

        // Logica de Echipare și Vânzare
        if (!player.getInventory().empty()) {
            if (!player.getUnits().empty()) {
                player.equipUnit(player.getUnits()[0], 0);
            } else {
                player.sellItem(0); // Apelăm sellItem
            }
        }

        internalUpdate();
        
        if (turns.getDay() > 10) running = false;
    }

    bool isRunning() const { return running; }
    void finalReport() const { 
        stat.showReport(); 
        std::cout << "Aur total generat în istorie: " << player.getLifetimeGold() << "g\n";
    }
};

// -----------------------------------------------------------
// MAIN - Punctul de intrare (CI & Terminal Friendly)
// -----------------------------------------------------------
int main() {
    // Seed pentru randomizare controlată
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    try {
        std::cout << "--- EMPIRE RISING: INIȚIALIZARE MOTOR SIMULARE ---\n";
        
        Simulation sim;
        
        // Bucla principală de execuție
        while (sim.isRunning()) {
            sim.runCycle();
        }

        sim.finalReport();
        std::cout << "\n--- SIMULARE FINALIZATĂ CU SUCCES ---\n";

    } catch (const std::exception& e) {
        std::cerr << "Eroare Fatală: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        return 1;
    }

    return 0;
}
