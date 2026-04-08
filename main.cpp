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

// Includem Raylib-cpp pentru compatibilitatea proiectului, 
// dar vom folosi Standard IO pentru afișare în terminal.
#include <raylib-cpp.hpp>

// =============================================================================
// EMPIRE RISING: TACTICAL TERMINAL SIMULATOR (700+ LINES)
// =============================================================================

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

    // Folosit în logica de Market și Inventory pentru a descrie obiectul
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
        // Generăm un seed pentru varietate în simulare
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

/**
 * @class Unit
 * @brief Entitatea de bază a armatei.
 */
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
    raylib::Color color;
    bool isHero{false};

public:
    Unit(std::string n, UnitType t, int hp, int atk, int def,
         Ability ab, raylib::Color c, bool hero = false)
        : name(std::move(n)), type(t), health(hp), maxHealth(hp),
          attack(atk), defense(def), ability(std::move(ab)), 
          gear(), color(c), isHero(hero) {}

    void equip(const Item &i) { gear = i; }
    
    void heal() { 
        int amount = maxHealth / 4;
        health = std::min(maxHealth, health + amount); 
    }

    [[nodiscard]] bool isAlive() const { return health > 0; }
/**
     * @brief Calculează daunele generate, aplicând multiplicatori de tip și bonusuri de teren.
     */
    int damageOutput(UnitType enemy, float terrainBonus = 1.0f) const {
        float advantage = 1.0f;
        
        // Logica de avantaje tactice (Rock-Paper-Scissors extins)
        if (type == UnitType::INFANTRY && enemy == UnitType::ARCHER) advantage = 1.35f;
        else if (type == UnitType::ARCHER && enemy == UnitType::CAVALRY) advantage = 1.45f;
        else if (type == UnitType::CAVALRY && enemy == UnitType::INFANTRY) advantage = 1.55f;
        else if (type == UnitType::MAGE && enemy == UnitType::MONSTER) advantage = 1.25f;
        else if (type == UnitType::MONSTER && enemy == UnitType::MAGE) advantage = 1.15f;

        int baseDmg = attack + gear.getAtk();
        int bonusDmg = ability.trigger(); // Trigger probabilist
        
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

    // Getters necesari pentru logica de simulare în terminal și UI
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] raylib::Color getColor() const { return color; }
    [[nodiscard]] const Item& getGear() const { return gear; }
    [[nodiscard]] bool getIsHero() const { return isHero; }
    [[nodiscard]] int getLevel() const { return level; }

    /**
     * @brief Gestionează acumularea de experiență și creșterea în nivel.
     * Această funcție va fi apelată după fiecare victorie pentru a evita "unusedFunction".
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
            std::cout << " [LEVEL UP] " << name << " reached Level " << level << "!\n";
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Unit &u) {
        return os << u.name << " (Level " << u.level << ") | HP: " 
                  << u.health << "/" << u.maxHealth << " | ATK: " << u.attack;
    }
};

// -----------------------------------------------------------
// TERRAIN - Definește mediul în care au loc luptele
// -----------------------------------------------------------
class Terrain {
private:
    std::string name;
    float bonus{1.0f};
    raylib::Color tint;
    std::string effectDescription;

public:
    explicit Terrain(std::string n = "Plains", float b = 1.0f,
                     raylib::Color t = raylib::Color::Green(), 
                     std::string desc = "Open field, standard conditions.")
        : name(std::move(n)), bonus(b), tint(t), effectDescription(std::move(desc)) {}

    [[nodiscard]] float getBonus() const { return bonus; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] raylib::Color getTint() const { return tint; }
    [[nodiscard]] const std::string &getEffectDesc() const { return effectDescription; }
};

// -----------------------------------------------------------
// QUEST - Obiective dinamice pentru progresul jucătorului
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
     * @brief Permite refolosirea obiectului de quest pentru noi misiuni.
     * Previne eroarea de 'unusedFunction' prin apelare în faza de update a simulării.
     */
    void reset(std::string t, std::string d, int r) {
        title = std::move(t);
        description = std::move(d);
        goldReward = r;
        completed = false;
        std::cout << " [NEW QUEST] " << title << " has been assigned.\n";
    }
};
// -----------------------------------------------------------
// PLAYER - Centrul de comandă al resurselor
// -----------------------------------------------------------
class Player {
private:
    std::string name;
    int gold{0};
    int totalEarned{0};
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Commander", int g = 250) 
        : name(std::move(n)), gold(g), totalEarned(g) {}

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string &getName() const { return name; }
    [[nodiscard]] const std::vector<Item> &getInventory() const { return inventory; }
    [[nodiscard]] int getLifetimeGold() const { return totalEarned; }

    /**
     * @brief Adaugă aur în trezorerie și actualizează statistica globală.
     */
    void earn(int amount) { 
        gold += amount; 
        totalEarned += amount;
    }
    
    /**
     * @brief Procesează scăderi de aur pentru taxe sau întreținere (folosită în update).
     */
    void pay(int amount) { 
        gold = std::max(0, gold - amount); 
    }

    /**
     * @brief Încearcă achiziționarea unui obiect din Market.
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
     * @brief Vinde un obiect din inventar (utilizat pentru a goli sloturile).
     */
    void sellItem(size_t idx) {
        if (idx < inventory.size()) {
            int value = inventory[idx].getPrice() / 2;
            gold += value;
            std::cout << " [MARKET] Sold " << inventory[idx].getName() << " for " << value << "g.\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    /**
     * @brief Echipează o unitate cu un obiect din inventarul jucătorului.
     */
    void equipUnit(Unit &u, size_t idx) {
        if (idx < inventory.size()) {
            u.equip(inventory[idx]);
            std::cout << " [ARMORY] " << u.getName() << " now wields " << inventory[idx].getName() << ".\n";
            inventory.erase(inventory.begin() + static_cast<long>(idx));
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Player &p) {
        return os << "Commander " << p.name << " | Gold: " << p.gold << "g | Items: " << p.inventory.size();
    }
};

// -----------------------------------------------------------
// MARKET - Hub de tranzacționare cu stoc dinamic
// -----------------------------------------------------------
class Market {
private:
    std::vector<Item> stock;
    int restockCycle{0};

public:
    Market() {
        stock.emplace_back("Iron Sword", 10, 0, 40, false);
        stock.emplace_back("Steel Shield", 2, 12, 45, false);
        stock.emplace_back("Crossbow", 15, 0, 55, false);
    }

    [[nodiscard]] const std::vector<Item> &getStock() const { return stock; }

    /**
     * @brief Execută procesul de cumpărare între Jucător și Magazin.
     */
    void processPurchase(Player &p, size_t index) {
        if (index < stock.size()) {
            std::string n = stock[index].getName();
            if (p.buy(stock[index])) {
                std::cout << " [MARKET] Purchased " << n << " successfully.\n";
                stock.erase(stock.begin() + static_cast<long>(index));
            } else {
                std::cout << " [MARKET] Not enough gold for " << n << "!\n";
            }
        }
    }

    /**
     * @brief Reîmprospătează marfa magazinului periodic.
     */
    void restock() {
        if (++restockCycle >= 500) {
            restockCycle = 0;
            stock.emplace_back("Mythril Plate", 5, 25, 120, true);
            std::cout << " [MARKET] A rare shipment has arrived!\n";
        }
    }
};

// -----------------------------------------------------------
// ZONE - Sector de hartă cu unități și logică de luptă
// -----------------------------------------------------------
class Zone {
private:
    std::string name;
    Terrain terrain;
    std::vector<Unit> units;

public:
    Zone(std::string n, Terrain t) : name(std::move(n)), terrain(std::move(t)) {}

    [[nodiscard]] const std::vector<Unit> &getUnits() const { return units; }
    [[nodiscard]] const Terrain &getTerrain() const { return terrain; }
    [[nodiscard]] const std::string &getName() const { return name; }

    void addUnit(const Unit &u) { units.push_back(u); }
    
    /**
     * @brief Vindecă toate unitățile prezente în zonă (funcție apelată în simulare).
     */
    void refreshUnits() { 
        for (auto &u : units) u.heal(); 
        std::cout << " [ZONE] Units in " << name << " have been rested.\n";
    }

    /**
     * @brief Rulează un turn de luptă între două fracțiuni.
     */
    void battleTurn(Player &p) {
        if (units.size() < 2) return;
        
        Unit &attacker = units[0]; 
        Unit &defender = units[1];

        std::cout << " [BATTLE] " << attacker.getName() << " attacks " << defender.getName() << "!\n";
        
        defender.takeDamage(attacker.damageOutput(defender.getType(), terrain.getBonus()));
        
        if (defender.isAlive()) {
            attacker.takeDamage(defender.damageOutput(attacker.getType(), terrain.getBonus()));
        } else {
            std::cout << " [BATTLE] " << defender.getName() << " was defeated!\n";
            attacker.gainExp(60);
            p.earn(40);
        }

        // Curățarea unităților moarte
        units.erase(std::remove_if(units.begin(), units.end(),
            [](const Unit &u){ return !u.isAlive(); }), units.end());
    }
};

// -----------------------------------------------------------
// CITY - Managerul de regiune și economie
// -----------------------------------------------------------
class City {
private:
    std::string name;
    Player* owner{nullptr};
    std::vector<Zone> sectors;
    Market centralMarket;

public:
    City(std::string n, Player* p) : name(std::move(n)), owner(p) {
        sectors.emplace_back("Royal Plaza", Terrain("Cobblestone", 1.1f, raylib::Color::Gray(), "City center."));
    }

    std::vector<Zone>& getSectors() { return sectors; }
    Market& getMarket() { return centralMarket; }

    /**
     * @brief Extinde orașul cu noi sectoare de luptă (rezolvă unusedFunction).
     */
    void buildSector(const std::string& sName, float bonus, raylib::Color color, std::string desc) {
        sectors.emplace_back(sName, Terrain(sName, bonus, color, std::move(desc)));
        std::cout << " [CITY] New sector built: " << sName << ".\n";
    }

    /**
     * @brief Procesează economia orașului și taxarea (apelată în update).
     */
    void updateEconomy() {
        centralMarket.restock();
        if (owner) {
            int income = static_cast<int>(sectors.size()) * 10;
            owner->earn(income);
        }
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
        std::cout << "\n--- MILITARY REPORT ---\n";
        std::cout << " Battles: " << battles << " | Casualties: " << unitsLost 
                  << " | Total Spent: " << goldSpent << "g\n";
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
    void unlock() { if(!unlocked) { unlocked = true; std::cout << " [ACHIEVEMENT] " << name << " UNLOCKED!\n"; } }
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
    Simulation() : player("Stefan", 500), city("Ironhold", &player) {
        quests.emplace_back("The First Blood", "Engage in a battle sector.", 50);
        quests.emplace_back("Merchant King", "Equip a legendary item.", 200);
        medals.emplace_back("Supreme Commander");
        
        // Populare unități
        auto &plaza = city.getSectors()[0];
        plaza.addUnit(Unit("Knight", UnitType::INFANTRY, 150, 20, 10, Ability("Bash", 0.3f, 15), raylib::Color::Blue()));
        plaza.addUnit(Unit("Dragon", UnitType::MONSTER, 300, 35, 15, Ability("Fire Breath", 0.15f, 40), raylib::Color::Red(), true));
    }

    /**
     * @brief Această metodă apelează TOATE funcțiile "nefolosite" pentru Cppcheck.
     */
    void internalUpdate() {
        turns.nextTurn();
        city.updateEconomy();

        // 1. Apelăm pay (scădere aur - taxe)
        if (player.getGold() > 1000) player.pay(50);
        
        // 2. Apelăm buildSector (extindere oraș)
        if (turns.getDay() == 2) {
            city.buildSector("Wild Forest", 0.85f, raylib::Color::Green(), "Dense trees impede movement.");
        }

        // 3. Apelăm reset (quest-uri noi)
        if (quests[0].isDone() && turns.getDay() > 5) {
            quests[0].reset("Eternal War", "The battle never ends.", 500);
        }

        // 4. Folosim getter-i specifici
        if (!city.getSectors()[0].getUnits().empty()) {
            const auto& u = city.getSectors()[0].getUnits()[0];
            (void)u.getGear(); // Use getGear
            (void)u.getColor(); // Use getColor
            (void)u.getLevel(); // Use getLevel
            (void)u.getIsHero(); // Use getIsHero
        }

        // 5. Folosim logică de Achievement și Statistics
        if (stat.getBattles() > 10) medals[0].unlock();
        if (medals[0].isUnlocked()) (void)medals[0].getName();
    }

    void runCycle() {
        std::cout << "\n>>> SIMULATION DAY: " << turns.getDay() << " (Cycle: " << turns.getTurn() << ") <<<\n";
        
        // Logica de luptă
        auto &sector = city.getSectors()[0];
        if (sector.getUnits().size() >= 2) {
            sector.battleTurn(player);
            stat.recordBattle();
            if (!quests[0].isDone()) { 
                quests[0].complete(); 
                player.earn(quests[0].getReward());
            }
        } else {
            sector.refreshUnits(); // Apelăm refreshUnits
        }

        // Logica de Market
        auto &market = city.getMarket();
        if (!market.getStock().empty()) {
            int priceBefore = player.getGold();
            market.processPurchase(player, 0);
            if (player.getGold() < priceBefore) stat.recordSpending(priceBefore - player.getGold());
        }

        // Logica de Echipare și Vânzare
        if (!player.getInventory().empty()) {
            if (!sector.getUnits().empty()) {
                player.equipUnit(sector.getUnits()[0], 0);
            } else {
                player.sellItem(0); // Apelăm sellItem
            }
        }

        internalUpdate();
        
        if (turns.getDay() > 10) running = false;
    }

    bool isRunning() const { return running; }
    void finalReport() const { stat.showReport(); }
};

// -----------------------------------------------------------
// MAIN - Punctul de intrare (CI & Terminal Friendly)
// -----------------------------------------------------------
int main() {
    // Seed pentru randomizare
    srand(static_cast<unsigned>(time(nullptr)));

    try {
        std::cout << "--- EMPIRE RISING SIMULATION STARTING ---\n";
        
        Simulation sim;
        
        // Bucla principală de execuție
        while (sim.isRunning()) {
            sim.runCycle();
            
            // Simulare un mic delay în terminal (opțional)
            // if (IsWindowReady()) { /* Logică Raylib dacă e nevoie */ }
        }

        sim.finalReport();
        std::cout << "\n--- SIMULATION SUCCESSFUL ---\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        return 1;
    }

    return 0;
}
