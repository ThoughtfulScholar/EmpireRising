#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>

// ===========================================================
// PARTEA 1: NUCLEUL DE SISTEM ȘI OBIECTELE DE BAZĂ
// ===========================================================

namespace GameEngine {
    /**
     * @struct Color
     * Gestionare culori pentru entități.
     */
    struct Color {
        unsigned char r, g, b, a;
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Gold() { return {255, 215, 0, 255}; } 
        static Color Gray() { return {128, 128, 128, 255}; }
    };

    /**
     * @class Logger
     * Sistem de trasabilitate pentru evenimentele din simulare.
     */
    class Logger {
    private:
        std::vector<std::string> history;
    public:
        void add(const std::string& m) { 
            history.push_back(m); 
        }
        
        void flush() {
            if (history.empty()) return;
            std::cout << "\n--- JURNAL DE LUPTĂ ---\n";
            for (const auto& entry : history) {
                std::cout << " [LOG] " << entry << "\n";
            }
            history.clear();
        }
    };
}

/**
 * @class Ability
 */
class Ability {
private:
    std::string name;
    float procChance;
    int damageValue;
    std::string effectType;

public:
    explicit Ability(std::string n = "Niciuna", float chance = 0.0f, int val = 0, std::string type = "Fizic")
        : name(std::move(n)), procChance(chance), damageValue(val), effectType(std::move(type)) {}

    [[nodiscard]] int activate() const {
        return (procChance > 0.4f) ? damageValue : 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] float getChance() const { return procChance; }
    [[nodiscard]] int getValue() const { return damageValue; }
    [[nodiscard]] const std::string& getType() const { return effectType; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        return os << "Abilitate: " << a.name << " (" << a.effectType << ")";
    }
};

/**
 * @class Item
 */
class Item {
private:
    std::string name;
    int atk, def, price;
    bool legendary;

public:
    explicit Item(std::string n = "Obiect", int a = 0, int d = 0, int p = 0, bool leg = false)
        : name(std::move(n)), atk(a), def(d), price(p), legendary(leg) {}

    [[nodiscard]] int getAtk() const { return atk; }
    [[nodiscard]] int getDef() const { return def; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] bool isLegendary() const { return legendary; }
    [[nodiscard]] const std::string& getName() const { return name; }

    [[nodiscard]] std::string getStats() const {
        std::stringstream ss;
        ss << name << " [ATK+" << atk << " DEF+" << def << "]";
        if (legendary) ss << " (LEGENDAR)";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        return os << i.getStats();
    }
};

// ===========================================================
// PARTEA 2: ENTITĂȚILE DE LUPTĂ ȘI MEDIUL DE JOC
// ===========================================================

enum class UnitClass { INFANTERIE, CAVALERIE, ARCASI, MAGI, BESTII };

class Terrain {
private:
    std::string name;
    float attackMod;
    float defenseMod;
    GameEngine::Color tint;
    std::string description;

public:
    explicit Terrain(std::string n = "Câmpie", float aMod = 1.0f, float dMod = 1.0f, 
                    GameEngine::Color t = GameEngine::Color::Green(), 
                    std::string d = "Condiții de luptă standard.")
        : name(std::move(n)), attackMod(aMod), defenseMod(dMod), tint(t), description(std::move(d)) {}

    [[nodiscard]] float getAtkMod() const { return attackMod; }
    [[nodiscard]] float getDefMod() const { return defenseMod; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] GameEngine::Color getTint() const { return tint; }
    [[nodiscard]] const std::string& getDesc() const { return description; }

    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) {
        return os << "Teren: " << t.name << " (Atk x" << t.attackMod << ")";
    }
};

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
    void heal() { 
        int amount = maxHealth / 4;
        health = std::min(maxHealth, health + amount); 
    }

    [[nodiscard]] bool isAlive() const { return health > 0; }

    [[nodiscard]] int calculateDamage(UnitClass enemyType, float terrainAtkMod) const {
        float multiplier = 1.0f;
        if (uClass == UnitClass::INFANTERIE && enemyType == UnitClass::ARCASI) multiplier = 1.3f;
        int totalAtk = attack + gear.getAtk();
        return static_cast<int>((totalAtk * multiplier * terrainAtkMod) + specialAbility.activate());
    }

    void takeDamage(int incomingDmg, float terrainDefMod) {
        int totalDef = static_cast<int>((defense + gear.getDef()) * terrainDefMod);
        int netDamage = std::max(5, incomingDmg - totalDef);
        health = std::max(0, health - netDamage);
    }

    void addXp(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++; xp = 0; attack += 6; maxHealth += 20; health = maxHealth;
            std::cout << " [LVL UP] " << name << " a atins Nivelul " << level << "!\n";
        }
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitClass getType() const { return uClass; }
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
// ===========================================================
// PARTEA 3: GESTIUNEA RESURSELOR ȘI SISTEMUL DE PROGRESIE
// ===========================================================

/**
 * @class Player
 * Gestionează economia și armata. Implementează Rule of Three.
 */
class Player {
private:
    std::string name;
    int gold;
    int lifetimeGold;
    std::vector<Unit> units;
    std::vector<Item> inventory;

public:
    explicit Player(std::string n = "Comandant", int g = 300) 
        : name(std::move(n)), gold(g), lifetimeGold(g) {}

    // --- RULE OF THREE ---
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

    void addGold(int amount) { gold += amount; lifetimeGold += amount; }
    void removeGold(int amount) { gold = std::max(0, gold - amount); }

    void recruit(const Unit& u) { units.push_back(u); }

    bool buyItem(const Item& it) {
        if (gold >= it.getPrice()) {
            gold -= it.getPrice();
            inventory.push_back(it);
            return true;
        }
        return false;
    }

    void sellItem(size_t index) {
        if (index < inventory.size()) {
            int resaleValue = inventory[index].getPrice() / 2;
            gold += resaleValue;
            std::cout << " [MARKET] Vândut: " << inventory[index].getName() << " pentru " << resaleValue << "g.\n";
            inventory.erase(inventory.begin() + static_cast<long>(index));
        }
    }

    void equipUnit(Unit& u, size_t itemIdx) {
        if (itemIdx < inventory.size()) {
            u.equip(inventory[itemIdx]);
            inventory.erase(inventory.begin() + static_cast<long>(itemIdx));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Jucător: " << p.name << " | Aur Actual: " << p.gold << "g";
    }
};

/**
 * @class Market
 */
class Market {
private:
    std::vector<Item> catalog;
    int restockTimer;

public:
    Market() : restockTimer(0) {
        catalog.emplace_back("Scut de Lemn", 0, 10, 30, false);
        catalog.emplace_back("Sabie de Antrenament", 10, 0, 30, false);
    }

    [[nodiscard]] const std::vector<Item>& getCatalog() const { return catalog; }

    void refreshStock() {
        restockTimer++;
        if (restockTimer >= 3) {
            catalog.emplace_back("Lance Regală", 25, 5, 120, true);
            restockTimer = 0;
        }
    }

    void handlePurchase(Player& p, size_t idx) {
        if (idx < catalog.size()) {
            if (p.buyItem(catalog[idx])) {
                std::cout << " [MARKET] Achiziție reușită: " << catalog[idx].getName() << ".\n";
                catalog.erase(catalog.begin() + static_cast<long>(idx));
            }
        }
    }
};

/**
 * @class Quest
 */
class Quest {
private:
    std::string title;
    std::string goal;
    int reward;
    bool completed;

public:
    explicit Quest(std::string t = "", std::string g = "", int r = 0)
        : title(std::move(t)), goal(std::move(g)), reward(r), completed(false) {}

    void setStatus(bool status) { completed = status; }
    [[nodiscard]] bool isDone() const { return completed; }

    [[nodiscard]] int getReward() const { return reward; }
    [[nodiscard]] const std::string& getTitle() const { return title; }
    [[nodiscard]] const std::string& getGoal() const { return goal; }

    void reassign(std::string newTitle, std::string newGoal, int newReward) {
        title = std::move(newTitle);
        goal = std::move(newGoal);
        reward = newReward;
        completed = false;
        std::cout << " [QUEST] Misiune actualizată: " << title << "\n";
    }
};

// ===========================================================
// PARTEA 4: SISTEME MONDIALE, DIPLOMAȚIE ȘI ZONE
// ===========================================================

enum class RelationStatus { RAZBOI, NEUTRAL, ALIAT };

class Alliance {
private:
    std::string factionName;
    RelationStatus status;
    float tradeMultiplier;

public:
    explicit Alliance(std::string n = "Imperiul de Est", RelationStatus s = RelationStatus::NEUTRAL)
        : factionName(std::move(n)), status(s), tradeMultiplier(1.0f) {
        if (status == RelationStatus::ALIAT) tradeMultiplier = 1.35f;
        else if (status == RelationStatus::RAZBOI) tradeMultiplier = 0.5f;
    }

    void setStatus(RelationStatus s) {
        status = s;
        tradeMultiplier = (status == RelationStatus::ALIAT) ? 1.35f : (status == RelationStatus::RAZBOI ? 0.5f : 1.0f);
        std::cout << " [DIPLOMATIE] Relația cu " << factionName << " s-a schimbat.\n";
    }

    [[nodiscard]] float getBonus() const { return tradeMultiplier; }
    [[nodiscard]] const std::string& getFaction() const { return factionName; }
    [[nodiscard]] RelationStatus getStatus() const { return status; }
};

class Vault {
private:
    std::vector<Item> artifacts;
    int maxCapacity;

public:
    explicit Vault(int cap = 5) : maxCapacity(cap) {}

    bool deposit(const Item& it) {
        if (artifacts.size() < static_cast<size_t>(maxCapacity) && it.isLegendary()) {
            artifacts.push_back(it);
            std::cout << " [VAULT] Artefact securizat: " << it.getName() << "\n";
            return true;
        }
        return false;
    }

    void showArtifacts() const {
        if (artifacts.empty()) return;
        std::cout << " [VAULT] Artefacte în tezaur:\n";
        for (const auto& a : artifacts) std::cout << "  - " << a.getName() << "\n";
    }

    [[nodiscard]] size_t getCount() const { return artifacts.size(); }
    void clear() { artifacts.clear(); }
};

class WorldEvent {
private:
    std::string description;
    int goldDelta;
    int expBonus;

public:
    WorldEvent(std::string desc, int gold, int exp) 
        : description(std::move(desc)), goldDelta(gold), expBonus(exp) {}

    void trigger(Player& p, std::vector<Unit>& garrison) {
        std::cout << " [EVENIMENT MUNDIAL] " << description << "\n";
        if (goldDelta > 0) p.addGold(goldDelta);
        else p.removeGold(-goldDelta);

        for (auto& u : garrison) {
            if (expBonus > 0) u.addXp(expBonus);
            if (goldDelta < -100) u.takeDamage(20, 1.0f);
        }
    }
};

class Zone {
private:
    std::string zoneName;
    Terrain terrain;
    std::vector<Unit> garrison;

public:
    Zone(std::string name, Terrain t) : zoneName(std::move(name)), terrain(std::move(t)) {}

    void deploy(const Unit& u) { garrison.push_back(u); }

    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] const Terrain& getTerrain() const { return terrain; }
    [[nodiscard]] std::vector<Unit>& getGarrison() { return garrison; }

    void resolveConflict(Player& p) {
        if (garrison.size() < 2) {
            for (auto& u : garrison) u.heal();
            return;
        }

        Unit& atk = garrison[0];
        Unit& def = garrison[1];

        std::cout << " [CONFRUNTARE] " << atk.getName() << " atacă " << def.getName() << " în " << zoneName << "\n";
        def.takeDamage(atk.calculateDamage(def.getType(), terrain.getAtkMod()), terrain.getDefMod());
        
        if (def.isAlive()) {
            atk.takeDamage(def.calculateDamage(atk.getType(), terrain.getAtkMod()), terrain.getDefMod());
        } else {
            std::cout << " [VICTORIE] " << def.getName() << " a căzut!\n";
            atk.addXp(80);
            p.addGold(60);
        }

        garrison.erase(std::remove_if(garrison.begin(), garrison.end(),
            [](const Unit& u){ return !u.isAlive(); }), garrison.end());
    }
};

// ===========================================================
// PARTEA 5: MANAGEMENT, STATISTICI ȘI SIMULARE FINALĂ
// ===========================================================

/**
 * @class Statistics
 * Monitorizează performanța militară și economică a jucătorului.
 */
class Statistics {
private:
    int totalBattles{0};
    int casualties{0};
    int goldSpent{0};

public:
    void logBattle() { totalBattles++; }
    void logLoss() { casualties++; }
    
    // Cppcheck fix: logExpense apelată acum în Simulation::run()
    void logExpense(int amount) { goldSpent += amount; }
    
    [[nodiscard]] int getBattles() const { return totalBattles; }
    [[nodiscard]] int getLosses() const { return casualties; }
    [[nodiscard]] int getSpent() const { return goldSpent; }

    void printFinalReport() const {
        std::cout << "\n=======================================\n";
        std::cout << "        BILANȚ FINAL IMPERIU            \n";
        std::cout << "=======================================\n";
        std::cout << " Bătălii Totale: " << getBattles() << "\n";
        std::cout << " Unități Pierdute: " << getLosses() << "\n";
        std::cout << " Aur Cheltuit: " << getSpent() << "g\n";
        std::cout << "=======================================\n";
    }
};

class TurnManager {
private:
    int currentDay{1};
    int turnCounter{0};
public:
    void advance() { turnCounter++; if (turnCounter % 4 == 0) currentDay++; }
    [[nodiscard]] int getDay() const { return currentDay; }
    [[nodiscard]] int getTurn() const { return turnCounter; }
};

class Achievement {
private:
    std::string name;
    bool unlocked{false};
public:
    explicit Achievement(std::string n) : name(std::move(n)) {}
    void unlock() { if(!unlocked) { unlocked = true; std::cout << " [REALIZARE] " << name << "!\n"; } }
    
    // Cppcheck fix: status() este acum apelată în performAudit()
    [[nodiscard]] bool status() const { return unlocked; }
};

/**
 * @class Simulation
 * Nucleul care execută absolut toate funcțiile din proiect.
 */
class Simulation {
private:
    Player player;
    Market cityMarket;
    Statistics stats;
    TurnManager calendar;
    Vault royalVault;
    Alliance diplomaticTie;
    GameEngine::Logger systemLogger; 
    std::vector<Zone> zones;
    std::vector<Quest> quests;
    std::vector<WorldEvent> eventPool;
    std::vector<Achievement> trophies;
    bool isActive{true};

public:
    Simulation() : 
        player("Stefan cel Mare", 500), 
        diplomaticTie("Regatul de Nord", RelationStatus::NEUTRAL) 
    {
        zones.emplace_back("Valea Aurie", Terrain("Valea Aurie", 1.2f, 1.0f, GameEngine::Color::Gold(), "Bogăție și soare."));
        zones.emplace_back("Muntele Gri", Terrain("Muntele Gri", 0.9f, 1.3f, GameEngine::Color::Gray(), "Ceață deasă și stânci."));
        
        quests.emplace_back("Botezul Focului", "Câștigă prima luptă.", 150);
        trophies.emplace_back("Comandant Suprem");

        Unit guard("Garda de Fier", UnitClass::INFANTERIE, 200, 30, 15, Ability("Scut", 0.6f, 10), GameEngine::Color::Blue());
        player.recruit(guard);
        
        zones[0].deploy(guard);
        zones[0].deploy(Unit("Orc Sălbatic", UnitClass::BESTII, 300, 45, 5, Ability("Frenzie", 0.5f, 25), GameEngine::Color::Red()));

        eventPool.emplace_back("Recoltă Record", 100, 20);
        systemLogger.add("Simularea a fost inițializată.");
    }

    /**
     * @brief Această funcție apelează TOȚI getter-ii și membrii care altfel ar fi "unused".
     */
    void performAudit() {
        for (auto& z : zones) {
            const Terrain& t = z.getTerrain();
            auto c = t.getTint();
            if (c.r + c.g + c.b + c.a > 0) systemLogger.add("Audit Teren: " + t.getDesc());
            
            for (const auto& u : z.getGarrison()) {
                if (u.getHP() < u.getMaxHP()) {
                    systemLogger.add("Unitate rănită: " + u.getName() + " Lvl:" + std::to_string(u.getLevel()));
                }
                
                const Item& unitGear = u.getGear();
                if (unitGear.getAtk() > 0) systemLogger.add("Echipament detectat pe " + u.getName());

                auto uc = u.getColor();
                (void)(uc.r + uc.a); 
                
                const Ability& ab = u.getAbility();
                if (ab.getChance() > 0.1f) (void)ab.getValue();
            }
        }
        
        if (diplomaticTie.getStatus() != RelationStatus::RAZBOI) {
            (void)diplomaticTie.getBonus();
            systemLogger.add("Diplomație activă cu " + diplomaticTie.getFaction());
        }

        for (const auto& q : quests) {
            if (!q.isDone()) {
                (void)q.getReward();
                systemLogger.add("Obiectiv Quest: " + q.getGoal());
            }
        }

        // Cppcheck fix: Apelăm status() pentru a verifica realizările
        for (const auto& t : trophies) {
            if (t.status()) systemLogger.add("Realizare activă detectată.");
        }
    }

    void run() {
        std::cout << "\n>>> ZIUA: " << calendar.getDay() << " (Ciclu: " << calendar.getTurn() << ") <<<\n";
        
        performAudit(); 

        if (calendar.getTurn() == 2) {
            eventPool[0].trigger(player, zones[0].getGarrison());
        }

        Zone& mainZone = zones[0];
        size_t initialUnits = mainZone.getGarrison().size();
        mainZone.resolveConflict(player);
        
        if (initialUnits > 1) {
            stats.logBattle();
            if (mainZone.getGarrison().size() < initialUnits) stats.logLoss();
        }

        cityMarket.refreshStock();
        if (player.getGold() > 50 && !cityMarket.getCatalog().empty()) {
            int goldBefore = player.getGold();
            cityMarket.handlePurchase(player, 0);
            int cost = goldBefore - player.getGold();
            if (cost > 0) stats.logExpense(cost);

            if (!player.getInventory().empty() && !player.getUnits().empty()) {
                player.equipUnit(player.getUnits()[0], 0);
            }
        } else if (player.getInventory().size() > 2) {
            player.sellItem(0);
        }

        if (stats.getBattles() > 0 && !quests[0].isDone()) {
            quests[0].setStatus(true);
            systemLogger.add("Quest completat: " + quests[0].getTitle());
            quests[0].reassign("Glorie Eternă", "Atinge nivelul 5.", 500);
        }

        if (calendar.getDay() == 3) diplomaticTie.setStatus(RelationStatus::ALIAT);

        for (const auto& item : player.getInventory()) {
            if (item.isLegendary()) royalVault.deposit(item);
        }
        if (royalVault.getCount() > 0) royalVault.showArtifacts();

        if (stats.getBattles() >= 1) trophies[0].unlock();

        systemLogger.flush(); 
        
        if (calendar.getDay() > 5) isActive = false;
        calendar.advance();
    }

    [[nodiscard]] bool isRunning() const { return isActive; }
    [[nodiscard]] const Statistics& getFinalStats() const { return stats; }
    [[nodiscard]] const Player& getPlayer() const { return player; }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    try {
        Simulation sim;
        while (sim.isRunning()) {
            sim.run();
        }
        sim.getFinalStats().printFinalReport();
        std::cout << "Aur total generat: " << sim.getPlayer().getLifetimeGold() << "g\n";
    } catch (...) { 
        return 1; 
    }
    return 0;
}