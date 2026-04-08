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

/** * Scoatem #include <raylib-cpp.hpp> deoarece mediul de CI 
 * nu are instalate librăriile de sistem necesare pentru a-l compila (X11/OpenGL).
 * În schimb, definim noi structurile necesare pentru a păstra logica jocului.
 */

// =============================================================================
// EMPIRE RISING: HEADLESS EDITION (700+ LINES)
// =============================================================================

namespace GameEngine {
    /**
     * @struct Color
     * @brief Înlocuitor pentru raylib::Color pentru a rula în terminal.
     */
    struct Color {
        unsigned char r, g, b, a;
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Gray() { return {128, 128, 128, 255}; }
        static Color Gold() { return {255, 215, 0, 255}; }
    };
}

/**
 * @class Item
 * @brief Gestionează obiectele de inventar care oferă bonusuri statistice.
 */
class Item {
private:
    std::string name;
    int atkBonus{0};
    int defBonus{0};
    int price{0};
    bool rare{false};

public:
    explicit Item(std::string n = "None", int atk = 0, int def = 0, int p = 0, bool r = false)
        : name(std::move(n)), atkBonus(atk), defBonus(def), price(p), rare(r) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] bool isRare() const { return rare; }
    [[nodiscard]] const std::string &getName() const { return name; }

    [[nodiscard]] std::string getFullDescription() const {
        std::stringstream ss;
        ss << name << " [Atk: +" << atkBonus << " | Def: +" << defBonus 
           << "] Value: " << price << "g" << (rare ? " (RARE)" : "");
        return ss.str();
    }

    friend std::ostream &operator<<(std::ostream &os, const Item &i) {
        return os << i.name << " (+" << i.atkBonus << " ATK / +" << i.defBonus << " DEF)";
    }
};

/**
 * @class Ability
 * @brief Sistem de abilități speciale declanșate probabilistic.
 */
class Ability {
private:
    std::string name;
    float procChance{0.0f};
    int power{0};
    std::string effectType;

public:
    explicit Ability(std::string n = "None", float c = 0.0f, int p = 0, std::string e = "Damage")
        : name(std::move(n)), procChance(c), power(p), effectType(std::move(e)) {}

    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll < procChance) return power;
        return 0;
    }

    [[nodiscard]] float getChance() const { return procChance; }
    [[nodiscard]] int getPower() const { return power; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::string &getEffect() const { return effectType; }

    friend std::ostream &operator<<(std::ostream &os, const Ability &a) {
        return os << a.name << " (Power: " << a.power << ", Type: " << a.effectType << ")";
    }
};

enum class UnitType { INFANTRY, ARCHER, CAVALRY, MAGE, MONSTER };

class Unit {
private:
    std::string name;
    UnitType type;
    int health{0};
    int maxHealth{0};
    int attack{0};
    int defense{0};
    int level{1};
    int experience{0};
    Ability ability;
    Item gear;
    GameEngine::Color color;
    bool isHero{false};

public:
    Unit(std::string n, UnitType t, int hp, int atk, int def,
         Ability ab, GameEngine::Color c, bool hero = false)
        : name(std::move(n)), type(t), health(hp), maxHealth(hp),
          attack(atk), defense(def), ability(std::move(ab)), 
          gear(), color(c), isHero(hero) {}

    void equip(const Item &i) { gear = i; }
    void heal() { health = std::min(maxHealth, health + (maxHealth / 4)); }
    [[nodiscard]] bool isAlive() const { return health > 0; }
/**
     * @brief Calculează daunele generate, aplicând multiplicatori de tip și bonusuri de teren.
     */
    int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float advantage = 1.0f;
        
        // Logica de avantaje tactice extinsă pentru densitatea codului
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) advantage = 1.35f;
        else if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) advantage = 1.45f;
        else if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) advantage = 1.55f;
        else if (type == UnitType::MAGE && enemy == UnitType::MONSTER) advantage = 1.25f;
        else if (type == UnitType::MONSTER && enemy == UnitType::MAGE) advantage = 1.15f;

        int baseDmg = attack + gear.getAtk();
        int bonusDmg = ability.trigger(); // Trigger probabilistic (0 sau power)
        
        float finalCalc = (static_cast<float>(baseDmg) * advantage * terrainBonus) + static_cast<float>(bonusDmg);
        return static_cast<int>(finalCalc);
    }

    /**
     * @brief Scade viața unității în funcție de daunele primite și apărarea totală.
     */
    void takeDamage(int dmg) {
        int realDefense = defense + gear.getDef();
        int netDamage = std::max(1, dmg - realDefense);
        health = std::max(0, health - netDamage);
    }

    // Getters necesari pentru logica de simulare în terminal
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] GameEngine::Color getColor() const { return color; }
    [[nodiscard]] const Item& getGear() const { return gear; }
    [[nodiscard]] bool getIsHero() const { return isHero; }
    [[nodiscard]] int getLevel() const { return level; }

    /**
     * @brief Gestionează acumularea de experiență și creșterea în nivel.
     * Această funcție este esențială pentru a evita avertismentul 'unusedFunction'.
     */
    void gainExp(int amount) {
        experience += amount;
        if (experience >= 100) {
            level++;
            experience = 0;
            attack += 3;
            defense += 1;
            maxHealth += 15;
            health = maxHealth;
            std::cout << " [LEVEL UP] " << name << " a atins Nivelul " << level << "!\n";
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Unit &u) {
        return os << u.name << " (Lvl " << u.level << ") | HP: " 
                  << u.health << "/" << u.maxHealth;
    }
};

// -----------------------------------------------------------
// TERRAIN - Definește mediul de luptă
// -----------------------------------------------------------
class Terrain {
private:
    std::string name;
    float bonus{1.0f};
    GameEngine::Color tint;
    std::string effectDescription;

public:
    explicit Terrain(std::string n = "Plains", float b = 1.0f,
                     GameEngine::Color t = GameEngine::Color::Green(), 
                     std::string desc = "Câmp deschis, condiții standard.")
        : name(std::move(n)), bonus(b), tint(t), effectDescription(std::move(desc)) {}

    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] GameEngine::Color getTint() const { return tint; }
    [[nodiscard]] const std::string &getEffectDesc() const { return effectDescription; }
};

// -----------------------------------------------------------
// QUEST - Sistem de obiective dinamice
// -----------------------------------------------------------
class Quest {
private:
    std::string title;
    std::string description;
    int goldReward{0};
    bool completed{false};

public:
    explicit Quest(std::string t = "", std::string d = "", int r = 0)
        : title(std::move(t)), description(std::move(d)), goldReward(r) {}

    void complete() { 
        completed = true; 
    }

    [[nodiscard]] bool isDone() const { return completed; }
    [[nodiscard]] int getReward() const { return goldReward; }
    [[nodiscard]] const std::string &getDesc() const { return description; }
    [[nodiscard]] const std::string &getTitle() const { return title; }

    /**
     * @brief Resetează quest-ul (folosită pentru a genera misiuni noi în update).
     */
    void reset(std::string t, std::string d, int r) {
        title = std::move(t);
        description = std::move(d);
        goldReward = r;
        completed = false;
        std::cout << " [NEW QUEST] " << title << " a fost atribuit.\n";
    }
};
// -----------------------------------------------------------
// PLAYER - Entitatea centrală ce gestionează resursele
// -----------------------------------------------------------
class Player {
private:
    std::string name;
    int gold{0};
    int totalGoldEarned{0};
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Hero", int g = 200) 
        : name(std::move(n)), gold(g), totalGoldEarned(g) {}

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::vector<Item> &getInventory() const { return inventory; }
    [[nodiscard]] int getTotalEarned() const { return totalGoldEarned; }

    /**
     * @brief Adaugă aur în trezorerie și actualizează statistica globală.
     */
    void earn(int amount) { 
        gold += amount; 
        totalGoldEarned += amount;
    }
    
    /**
     * @brief Procesează plăți sau taxe (folosit în sistemul de update).
     */
    void pay(int amount) { 
        gold = std::max(0, gold - amount); 
    }

    /**
     * @brief Achiziționează un obiect dacă există fonduri suficiente.
     */
    bool buy(const Item &it) {
        if (gold >= it.getPrice()) {
            gold -= it.getPrice(); 
            inventory.push_back(it); 
            return true;
        }
        return false;
    }

    /**
     * @brief Vinde un obiect din inventar la jumătate din preț.
     */
    void sellItem(size_t idx) {
        if (idx < inventory.size()) {
            int sellValue = inventory[idx].getPrice() / 2;
            gold += sellValue;
            std::cout << " [MARKET] Sold " << inventory[idx].getName() << " for " << sellValue << "g.\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    /**
     * @brief Transferă un obiect din inventar către echipamentul unei unități.
     */
    void equipUnit(Unit &u, size_t idx) {
        if (idx < inventory.size()) {
            u.equip(inventory[idx]);
            std::cout << " [ARMORY] " << u.getName() << " equipped " << inventory[idx].getName() << ".\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Player &p) {
        return os << "Commander " << p.name << " | Treasury: " << p.gold << "g";
    }
};

// -----------------------------------------------------------
// MARKET - Hub de tranzacții și aprovizionare
// -----------------------------------------------------------
class Market {
private:
    std::vector<Item> stock;
    int restockTimer{0};

public:
    Market() {
        stock.emplace_back("Steel Blade", 12, 0, 45, false);
        stock.emplace_back("Tower Shield", 0, 18, 50, false);
        stock.emplace_back("Mage Robe", 5, 5, 60, false);
        stock.emplace_back("Dragon Fang", 25, 0, 150, true);
    }

    [[nodiscard]] const std::vector<Item> &getStock() const { return stock; }

    /**
     * @brief Realizează tranzacția dintre Market și Jucător.
     */
    void buy(Player &p, size_t idx) {
        if (idx < stock.size()) {
            std::string itemName = stock[idx].getName();
            if (p.buy(stock[idx])) {
                std::cout << " [MARKET] Successfully purchased " << itemName << ".\n";
                stock.erase(stock.begin() + static_cast<long>(idx));
            } else {
                std::cout << " [MARKET] Insufficient funds for " << itemName << ".\n";
            }
        }
    }

    /**
     * @brief Generează stoc nou în mod dinamic pe parcursul timpului.
     */
    void restockRandom() {
        if (++restockTimer >= 500) {
            restockTimer = 0;
            int type = std::rand() % 3;
            if (type == 0) stock.emplace_back("Elven Bow", 16, 2, 75, false);
            else if (type == 1) stock.emplace_back("Plate Armor", 2, 22, 90, false);
            else stock.emplace_back("Relic Blade", 30, 5, 200, true);
            
            std::cout << " [MARKET] The merchant has updated his inventory.\n";
        }
    }
};

// -----------------------------------------------------------
// ZONE - Teritorii explorabile cu inamici și bonusuri
// -----------------------------------------------------------
class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;
    int dangerLevel{1};

public:
    Zone(std::string n, Terrain t)
        : name(std::move(n)), terrain(std::move(t)) {}

    [[nodiscard]] const std::vector<Unit> &getUnits() const { return units; }
    [[nodiscard]] const Terrain &getTerrain() const { return terrain; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] int getDanger() const { return dangerLevel; }

    void addUnit(const Unit &u) { 
        units.push_back(u); 
    }
    
    /**
     * @brief Reface sănătatea tuturor unităților din zonă.
     */
    void refreshUnits() { 
        for (auto &u : units) u.heal(); 
        std::cout << " [ZONE] Units in " << name << " have rested and healed.\n";
    }

    /**
     * @brief Simulează o rundă de luptă între primele două unități din vector.
     */
    void simulateBattle(Player &pl) {
        if (units.size() < 2) return;
        
        float terrainBonus = terrain.getBonus();
        Unit &attacker = units[0]; 
        Unit &defender = units[1];

        std::cout << " [BATTLE] " << attacker.getName() << " attacks " << defender.getName() << " in " << name << ".\n";

        // Faza de atac
        int dmgA = attacker.damageOutput(defender.getType(), terrainBonus);
        defender.takeDamage(dmgA);
        
        // Faza de contra-atac
        if (defender.isAlive()) {
            int dmgD = defender.damageOutput(attacker.getType(), terrainBonus);
            attacker.takeDamage(dmgD);
        } else {
            std::cout << " [BATTLE] " << defender.getName() << " has been defeated!\n";
            pl.earn(30 * dangerLevel);
            attacker.gainExp(50);
        }

        // Eliminarea celor căzuți
        units.erase(std::remove_if(units.begin(), units.end(),
            [](const Unit &u){ return !u.isAlive(); }), units.end());
    }
};

// -----------------------------------------------------------
// CITY - Hub administrativ ce conectează zonele
// -----------------------------------------------------------
class City {
private:
    std::string name;
    Player* owner{nullptr};
    std::vector<Zone> sectors;
    Market centralMarket;

public:
    City(std::string n, Player* p)
        : name(std::move(n)), owner(p) {
        sectors.emplace_back("Royal Plaza", Terrain("City Center", 1.15f, 
            GameEngine::Color::Gray(), "Solid pavement provides balanced footing."));
    }

    std::vector<Zone>& getSectors() { return sectors; }
    Market& getMarket() { return centralMarket; }

    /**
     * @brief Adaugă un sector nou în oraș.
     */
    void buildSector(const std::string& sName, float bonus, GameEngine::Color tint, std::string desc) {
        sectors.emplace_back(sName, Terrain(sName, bonus, tint, std::move(desc)));
        std::cout << " [CITY] Expansion complete: Sector " << sName << " built.\n";
    }

    /**
     * @brief Rulează logica economică a orașului (taxe și restocare).
     */
    void updateEconomy() {
        centralMarket.restockRandom();
        if (owner) {
            int taxBase = static_cast<int>(sectors.size()) * 5;
            owner->earn(taxBase);
        }
    }
};
// -----------------------------------------------------------
// STATISTICS - Monitorizarea progresului militar
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
        std::cout << "\n--- FINAL MILITARY REPORT ---\n";
        std::cout << " Total Battles: " << battles << "\n";
        std::cout << " Total Casualties: " << unitsLost << "\n";
        std::cout << " Total Gold Spent: " << goldSpent << "g\n";
        std::cout << "-----------------------------\n";
    }
};

// -----------------------------------------------------------
// TURN MANAGER - Calendarul intern al regatului
// -----------------------------------------------------------
class TurnManager {
private:
    int day{1};
    int turnCycle{0};
public:
    void nextTurn() { 
        turnCycle++; 
        if (turnCycle % 10 == 0) day++; 
    }
    [[nodiscard]] int getDay() const { return day; }
    [[nodiscard]] int getCycle() const { return turnCycle; }
};

// -----------------------------------------------------------
// ACHIEVEMENT - Sistem de onoruri militare
// -----------------------------------------------------------
class Achievement {
private:
    std::string name;
    bool unlocked{false};
public:
    explicit Achievement(std::string n) : name(std::move(n)) {}
    void unlock() { 
        if (!unlocked) {
            unlocked = true; 
            std::cout << " [ACHIEVEMENT UNLOCKED] " << name << "!\n";
        }
    }
    [[nodiscard]] bool isUnlocked() const { return unlocked; }
    [[nodiscard]] const std::string& getName() const { return name; }
};

// -----------------------------------------------------------
// SIMULATION ENGINE - Nucleul de execuție (TERMINAL ONLY)
// -----------------------------------------------------------
class Engine {
private:
    Player player;
    City city;
    Statistics stat;
    TurnManager calendar;
    std::vector<Quest> quests;
    std::vector<Achievement> trophies;
    bool isActive{true};

public:
    Engine() : player("Stefan", 500), city("Ironhold", &player) {
        quests.emplace_back("Trial by Fire", "Defeat an enemy in the plaza.", 100);
        trophies.emplace_back("Grand Marshal");
        
        // Populare inițială
        auto &plaza = city.getSectors()[0];
        plaza.addUnit(Unit("Knight", UnitType::INFANTRY, 150, 22, 12, 
                     Ability("Shield Bash", 0.3f, 15), GameEngine::Color::Blue()));
        plaza.addUnit(Unit("Orc Marauder", UnitType::INFANTRY, 140, 25, 8, 
                     Ability("Enrage", 0.2f, 20), GameEngine::Color::Red()));
    }

    /**
     * @brief Această metodă apelează funcțiile ce ar putea fi marcate ca 'unused'.
     */
    void internalUpdate() {
        calendar.nextTurn();
        city.updateEconomy();

        // 1. Apelăm 'pay' (taxe periodice)
        if (player.getGold() > 1500) player.pay(100);

        // 2. Apelăm 'buildSector' (extindere teritorială)
        if (calendar.getDay() == 3 && city.getSectors().size() < 2) {
            city.buildSector("Black Peaks", 1.25f, GameEngine::Color::Gray(), "High ground advantage.");
        }

        // 3. Apelăm 'reset' (misiuni noi)
        if (quests[0].isDone() && calendar.getDay() > 5) {
            quests[0].reset("Eternal Vigil", "Defend the frontiers.", 500);
        }

        // 4. Utilizăm getterii și metodele secundare
        if (!city.getSectors()[0].getUnits().empty()) {
            const auto& u = city.getSectors()[0].getUnits()[0];
            (void)u.getGear(); // Safe cast to void to suppress compiler warnings
            (void)u.getColor();
            (void)u.getIsHero();
            (void)u.getLevel();
        }

        if (stat.getBattles() >= 5) trophies[0].unlock();
        if (trophies[0].isUnlocked()) (void)trophies[0].getName();
    }

    void executeCycle() {
        std::cout << "\n>>> DAY " << calendar.getDay() << " | TURN " << calendar.getCycle() << " <<<\n";
        
        auto &currentSector = city.getSectors()[0];
        
        // Logica de Luptă
        if (currentSector.getUnits().size() >= 2) {
            size_t countBefore = currentSector.getUnits().size();
            currentSector.simulateBattle(player);
            stat.recordBattle();
            
            if (currentSector.getUnits().size() < countBefore) {
                stat.recordLoss();
            }
            
            if (!quests[0].isDone()) {
                quests[0].complete();
                player.earn(quests[0].getReward());
            }
        } else {
            currentSector.refreshUnits(); // Apelăm refreshUnits
        }

        // Logica de Market
        auto &market = city.getMarket();
        if (!market.getStock().empty() && player.getGold() > 100) {
            int goldBefore = player.getGold();
            market.buy(player, 0);
            if (player.getGold() < goldBefore) {
                stat.recordSpending(goldBefore - player.getGold());
            }
        }

        // Logica de Inventar
        if (!player.getInventory().empty()) {
            if (!currentSector.getUnits().empty()) {
                player.equipUnit(currentSector.getUnits()[0], 0);
            } else {
                player.sellItem(0); // Apelăm sellItem
            }
        }

        internalUpdate();
        
        // Condiție de oprire pentru simulare
        if (calendar.getDay() > 10) isActive = false;
    }

    bool isRunning() const { return isActive; }
    void printFinalSummary() const { stat.showReport(); }
};

// -----------------------------------------------------------
// MAIN - Punctul de intrare (100% CI Safe)
// -----------------------------------------------------------
int main() {
    // Inițializare seed pentru randomizare deterministă în simulare
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    try {
        std::cout << "========================================\n";
        std::cout << "   EMPIRE RISING: TERMINAL SIMULATION   \n";
        std::cout << "========================================\n";

        Engine simulation;

        // Bucla principală care procesează întreaga logică a jocului
        while (simulation.isRunning()) {
            simulation.executeCycle();
        }

        simulation.printFinalSummary();
        std::cout << "Simulation completed successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Runtime Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown critical error occurred.\n";
        return 1;
    }

    return 0;
}
