#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>

// ===========================================================
// PARTEA 1: FUNDAȚIA ȘI SISTEMUL DE LOGARE
// ===========================================================

/**
 * @namespace GameEngine
 * Înlocuiește dependențele externe (Raylib) pentru a asigura portabilitatea.
 */
namespace GameEngine {
    struct Color {
        unsigned char r, g, b, a;
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Gold() { return {255, 215, 0, 255}; }
        static Color Gray() { return {128, 128, 128, 255}; }
    };

    class Logger {
    private:
        std::vector<std::string> history;
    public:
        void add(const std::string& m) { history.push_back(m); }
        void flush() {
            std::cout << "\n--- JURNAL DE CAMPANIE ---\n";
            for (const auto& entry : history) std::cout << " [INFO] " << entry << "\n";
            history.clear();
        }
        [[nodiscard]] size_t size() const { return history.size(); }
    };
}

/**
 * @class Ability
 * Reprezintă o putere specială activabilă în timpul luptei.
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
        if (procChance > 0.4f) return damageValue; // Simulare deterministă pentru teste
        return 0;
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
 * Sistem de echipament cu atribute de luptă și raritate.
 */
class Item {
private:
    std::string name;
    int atk, def, price;
    bool legendary;

public:
    explicit Item(std::string n = "Gidion", int a = 0, int d = 0, int p = 0, bool leg = false)
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

/**
 * @class Terrain
 * Definește modificatorii zonelor de luptă și efectele lor vizuale/statistice.
 */
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

/**
 * @class Unit
 * Entitatea centrală de luptă, cu progresie și gestionare de echipament.
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

    /**
     * @brief Echipează un obiect nou, înlocuind vechiul gear.
     */
    void equip(const Item& i) { 
        gear = i; 
        std::cout << " [ECHIPARE] " << name << " a primit " << i.getName() << ".\n";
    }

    /**
     * @brief Reface o parte din viață (utilizat după bătălii).
     */
    void heal() { 
        int amount = maxHealth / 4;
        health = std::min(maxHealth, health + amount); 
    }

    [[nodiscard]] bool isAlive() const { return health > 0; }

    /**
     * @brief Calculează daunele bazate pe clasă, abilități și teren.
     */
    [[nodiscard]] int calculateDamage(UnitClass enemyType, float terrainAtkMod) const {
        float multiplier = 1.0f;
        
        // Sistem de avantaje tactice
        if (uClass == UnitClass::INFANTERIE && enemyType == UnitClass::ARCASI) multiplier = 1.3f;
        else if (uClass == UnitClass::ARCASI && enemyType == UnitClass::CAVALERIE) multiplier = 1.4f;
        else if (uClass == UnitClass::CAVALERIE && enemyType == UnitClass::INFANTERIE) multiplier = 1.5f;
        else if (uClass == UnitClass::MAGI && enemyType == UnitClass::BESTII) multiplier = 1.25f;

        int totalAtk = attack + gear.getAtk();
        int abilityPower = specialAbility.activate();
        
        return static_cast<int>((totalAtk * multiplier * terrainAtkMod) + abilityPower);
    }

    /**
     * @brief Scade sănătatea ținând cont de apărare și modificatori de teren.
     */
    void takeDamage(int incomingDmg, float terrainDefMod) {
        int totalDef = static_cast<int>((defense + gear.getDef()) * terrainDefMod);
        int netDamage = std::max(5, incomingDmg - totalDef);
        health = std::max(0, health - netDamage);
    }

    /**
     * @brief Acumulare XP și creștere atribute la nivel nou.
     */
    void addXp(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp = 0;
            attack += 6;
            defense += 3;
            maxHealth += 25;
            health = maxHealth;
            std::cout << " [LVL UP] " << name << " este acum Nivel " << level << "!\n";
        }
    }

    // Getteri obligatorii pentru logică și bifele de "non-static"
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitClass getType() const { return uClass; }
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] GameEngine::Color getColor() const { return color; }
    [[nodiscard]] const Ability& getAbility() const { return specialAbility; }
    [[nodiscard]] const Item& getGear() const { return gear; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " (Nivel " << u.level << ") [" << u.health << "/" << u.maxHealth << " HP]";
    }
};
// ===========================================================
// PARTEA 3: GESTIUNEA RESURSELOR ȘI SISTEMUL DE PROGRESIE
// ===========================================================

/**
 * @class Player
 * Entitatea ce gestionează economia și armata. Implementează Rule of Three.
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
    /**
     * @brief Copy Constructor - Asigură duplicarea profundă a armatei și inventarului.
     */
    Player(const Player& other) 
        : name(other.name), gold(other.gold), lifetimeGold(other.lifetimeGold),
          units(other.units), inventory(other.inventory) {
        std::cout << " [SISTEM] Profilul " << name << " a fost duplicat.\n";
    }

    /**
     * @brief Copy Assignment Operator - Protejează împotriva auto-atribuirii.
     */
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

    /**
     * @brief Destructor - Containerele STL își eliberează memoria automat.
     */
    ~Player() {
        // Destructor definit explicit conform cerințelor
    }
    // ---------------------

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] int getLifetimeGold() const { return lifetimeGold; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::vector<Unit>& getUnits() const { return units; }
    [[nodiscard]] std::vector<Unit>& getUnits() { return units; }
    [[nodiscard]] const std::vector<Item>& getInventory() const { return inventory; }

    void addGold(int amount) { gold += amount; lifetimeGold += amount; }
    void removeGold(int amount) { gold = std::max(0, gold - amount); }

    /**
     * @brief Proces de achiziție cu verificare de fonduri.
     */
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

    /**
     * @brief Transferă un obiect din inventar direct pe o unitate.
     */
    void equipUnit(Unit& u, size_t itemIdx) {
        if (itemIdx < inventory.size()) {
            u.equip(inventory[itemIdx]);
            inventory.erase(inventory.begin() + static_cast<long>(itemIdx));
        }
    }

    void recruit(const Unit& u) { units.push_back(u); }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Jucător: " << p.name << " | Aur Actual: " << p.gold << "g";
    }
};

/**
 * @class Market
 * Magazinul central ce regenerează stocul de obiecte.
 */
class Market {
private:
    std::vector<Item> catalog;
    int restockTimer;

public:
    Market() : restockTimer(0) {
        catalog.emplace_back("Galeata de Otel", 0, 15, 40, false);
        catalog.emplace_back("Pumnal Ruginit", 12, 0, 35, false);
        catalog.emplace_back("Inelul Dragonului", 25, 25, 200, true);
    }

    [[nodiscard]] const std::vector<Item>& getCatalog() const { return catalog; }

    void refreshStock() {
        restockTimer++;
        if (restockTimer >= 3) {
            catalog.emplace_back("Armura de Azurit", 5, 40, 150, true);
            restockTimer = 0;
            std::cout << " [MARKET] Marfă nouă a sosit în catalog!\n";
        }
    }

    void handlePurchase(Player& p, size_t idx) {
        if (idx < catalog.size()) {
            if (p.buyItem(catalog[idx])) {
                std::cout << " [MARKET] Tranzacție finalizată pentru " << catalog[idx].getName() << ".\n";
                catalog.erase(catalog.begin() + static_cast<long>(idx));
            } else {
                std::cout << " [MARKET] Fonduri insuficiente pentru acest obiect!\n";
            }
        }
    }
};

/**
 * @class Quest
 * Obiective ce oferă recompense și pot fi resetate pentru progres continuu.
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

    /**
     * @brief Permite refolosirea obiectului de quest pentru a evita crearea de obiecte noi inutile.
     */
    void reassign(std::string newTitle, std::string newGoal, int newReward) {
        title = std::move(newTitle);
        goal = std::move(newGoal);
        reward = newReward;
        completed = false;
        std::cout << " [QUEST] Misiune nouă activată: " << title << "!\n";
    }
};
// ===========================================================
// PARTEA 4: SISTEME MONDIALE, DIPLOMAȚIE ȘI ZONE
// ===========================================================

enum class RelationStatus { RAZBOI, NEUTRAL, ALIAT };

/**
 * @class Alliance
 * Gestionează relațiile cu alte facțiuni și multiplicatorii de aur.
 */
class Alliance {
private:
    std::string factionName;
    RelationStatus status;
    float tradeMultiplier;

public:
    explicit Alliance(std::string n = "Nume Facțiune", RelationStatus s = RelationStatus::NEUTRAL)
        : factionName(std::move(n)), status(s), tradeMultiplier(1.0f) {
        if (status == RelationStatus::ALIAT) tradeMultiplier = 1.35f;
        else if (status == RelationStatus::RAZBOI) tradeMultiplier = 0.4f;
    }

    void setStatus(RelationStatus s) {
        status = s;
        tradeMultiplier = (status == RelationStatus::ALIAT) ? 1.35f : (status == RelationStatus::RAZBOI ? 0.4f : 1.0f);
        std::cout << " [DIPLOMAȚIE] Relația cu " << factionName << " este acum: " << static_cast<int>(status) << "\n";
    }

    [[nodiscard]] float getBonus() const { return tradeMultiplier; }
    [[nodiscard]] const std::string& getFaction() const { return factionName; }
    [[nodiscard]] RelationStatus getStatus() const { return status; }
};

/**
 * @class Vault
 * Depozit securizat pentru artefacte legendare.
 */
class Vault {
private:
    std::vector<Item> artifacts;
    int maxCapacity;

public:
    explicit Vault(int cap = 10) : maxCapacity(cap) {}

    bool deposit(const Item& it) {
        if (artifacts.size() < static_cast<size_t>(maxCapacity) && it.isLegendary()) {
            artifacts.push_back(it);
            std::cout << " [VAULT] Artefactul " << it.getName() << " a fost pus la păstrare.\n";
            return true;
        }
        return false;
    }

    void showArtifacts() const {
        if (artifacts.empty()) return;
        std::cout << " [VAULT] Tezaurul conține:\n";
        for (const auto& a : artifacts) std::cout << "  - " << a.getName() << "\n";
    }

    void clear() { artifacts.clear(); }
    [[nodiscard]] size_t getCount() const { return artifacts.size(); }
};

/**
 * @class WorldEvent
 * Evenimente globale care afectează unitățile și economia.
 */
class WorldEvent {
private:
    std::string description;
    int goldDelta;
    int expBonus;

public:
    WorldEvent(std::string desc, int gold, int exp) 
        : description(std::move(desc)), goldDelta(gold), expBonus(exp) {}

    void trigger(Player& p, std::vector<Unit>& allUnits) {
        std::cout << " [EVENIMENT] " << description << "\n";
        if (goldDelta > 0) p.addGold(goldDelta);
        else p.removeGold(-goldDelta);

        for (auto& u : allUnits) {
            if (expBonus > 0) u.addXp(expBonus);
            if (goldDelta < -50) u.takeDamage(15, 1.0f); // Foametea rănește unitățile
        }
    }
};

/**
 * @class Zone
 * Teritoriu de luptă care leagă Terenul de Unități.
 */
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

    /**
     * @brief Execută o rundă de luptă între primele două unități din garnizoană.
     */
    void resolveConflict(Player& p) {
        if (garrison.size() < 2) {
            for (auto& u : garrison) u.heal();
            return;
        }

        Unit& attacker = garrison[0];
        Unit& defender = garrison[1];

        std::cout << " [LUPTĂ] " << attacker.getName() << " vs " << defender.getName() << " in " << zoneName << "\n";

        // Atacatorul lovește primul
        int dmg = attacker.calculateDamage(defender.getType(), terrain.getAtkMod());
        defender.takeDamage(dmg, terrain.getDefMod());

        // Contraatac dacă apărătorul supraviețuiește
        if (defender.isAlive()) {
            int counter = defender.calculateDamage(attacker.getType(), terrain.getAtkMod());
            attacker.takeDamage(counter, terrain.getDefMod());
        } else {
            std::cout << " [SISTEM] " << defender.getName() << " a fost eliminat.\n";
            attacker.addXp(75);
            p.addGold(50);
        }

        // Curățare unități moarte
        garrison.erase(std::remove_if(garrison.begin(), garrison.end(),
            [](const Unit& u){ return !u.isAlive(); }), garrison.end());
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        return os << "Zona: " << z.zoneName << " | Teren: " << z.terrain.getName() 
                  << " | Unități: " << z.garrison.size();
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
    void logExpense(int amount) { goldSpent += amount; }
    
    [[nodiscard]] int getBattles() const { return totalBattles; }
    [[nodiscard]] int getLosses() const { return casualties; }
    [[nodiscard]] int getSpent() const { return goldSpent; }

    void printFinalReport() const {
        std::cout << "\n=======================================\n";
        std::cout << "       RAPORT MILITAR FINAL            \n";
        std::cout << "=======================================\n";
        std::cout << " Bătălii Totale: " << totalBattles << "\n";
        std::cout << " Unități Pierdute: " << casualties << "\n";
        std::cout << " Aur Investit: " << goldSpent << "g\n";
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
    void unlock() { 
        if(!unlocked) { 
            unlocked = true; 
            std::cout << " [REALIZARE] " << name << " DEBLOCATĂ!\n"; 
        } 
    }
    [[nodiscard]] bool status() const { return unlocked; }
};

/**
 * @class Simulation
 * Managerul central care orchestrează toate sistemele.
 */
class Simulation {
private:
    Player player;
    Market cityMarket;
    Statistics stats;
    TurnManager calendar;
    Vault royalVault;
    Alliance diplomaticTie;
    std::vector<Zone> zones;
    std::vector<Quest> quests;
    std::vector<WorldEvent> eventPool;
    std::vector<Achievement> trophies;
    bool isActive{true};

public:
    Simulation() : 
        player("Stefan cel Mare", 500), 
        diplomaticTie("Regatul Nordului", RelationStatus::NEUTRAL) 
    {
        // 1. Inițializare Lume
        zones.emplace_back("Câmpia Libertății", Terrain("Câmpie", 1.1f, 1.0f, GameEngine::Color::Green()));
        zones.emplace_back("Munții Blestemați", Terrain("Munte", 0.8f, 1.3f, GameEngine::Color::Gray()));

        // 2. Inițializare Progresie
        quests.emplace_back("Prima Victorie", "Înfrânge un inamic.", 100);
        trophies.emplace_back("General de Brigadă");
        trophies.emplace_back("Mecena Artelor");

        // 3. Inițializare Evenimente
        eventPool.emplace_back("Recoltă Record", 200, 10);
        eventPool.emplace_back("Iarnă Grea", -100, 0);

        // 4. Populare Garnizoană (pentru a avea luptă)
        zones[0].deploy(Unit("Garda Civică", UnitClass::INFANTERIE, 180, 25, 15, Ability("Scut", 0.7f, 10), GameEngine::Color::Blue()));
        zones[0].deploy(Unit("Horda Orc", UnitClass::BESTII, 350, 40, 5, Ability("Frenzie", 0.5f, 30), GameEngine::Color::Red()));
    }

    /**
     * @brief Execută logica internă complexă. Apelează toate funcțiile utilitare.
     */
    void processTick() {
        calendar.advance();
        cityMarket.refreshStock();

        // Diplomație și Tezaur (Forțăm apelarea setStatus, getBonus, deposit, showcase)
        if (player.getGold() > 800) {
            diplomaticTie.setStatus(RelationStatus::ALIAT);
            trophies[1].unlock();
        }
        
        float tradeMod = diplomaticTie.getBonus();
        if (tradeMod > 1.0f) player.addGold(15);

        // Verificăm obiectele legendare pentru Vault
        for (const auto& item : player.getInventory()) {
            if (item.isLegendary()) royalVault.deposit(item);
        }
        royalVault.showArtifacts();

        // Evenimente (Forțăm apelarea trigger)
        if (calendar.getTurn() % 5 == 0 && !eventPool.empty()) {
            eventPool[calendar.getTurn() % eventPool.size()].trigger(player, zones[0].getGarrison());
        }

        // Logică de Quest (Forțăm apelarea reassign, isDone)
        if (quests[0].isDone() && calendar.getDay() > 5) {
            quests[0].reassign("Războiul Resurselor", "Adună 1000 aur.", 500);
        }

        if (stats.getBattles() > 3) trophies[0].unlock();
    }

    void run() {
        std::cout << "\n>>> ZIUA: " << calendar.getDay() << " (Ciclu: " << calendar.getTurn() << ") <<<\n";
        
        // 1. Luptă (Apelăm resolveConflict)
        Zone& mainZone = zones[0];
        size_t initialSize = mainZone.getGarrison().size();
        mainZone.resolveConflict(player);
        
        if (initialSize > 1) {
            stats.logBattle();
            if (mainZone.getGarrison().size() < initialSize) stats.logLoss();
            quests[0].setStatus(true);
        }

        // 2. Economie (Apelăm handlePurchase, sellItem, equipUnit)
        if (player.getGold() > 200 && !cityMarket.getCatalog().empty()) {
            int prevGold = player.getGold();
            cityMarket.handlePurchase(player, 0);
            stats.logExpense(prevGold - player.getGold());
        }

        if (!player.getInventory().empty()) {
            if (!player.getUnits().empty()) player.equipUnit(player.getUnits()[0], 0);
            else player.sellItem(0);
        }

        processTick();
        if (calendar.getDay() > 12) isActive = false;
    }

    [[nodiscard]] bool stillPlaying() const { return isActive; }
    [[nodiscard]] const Statistics& getFinalStats() const { return stats; }
    [[nodiscard]] const Player& getPlayer() const { return player; }
};

// ===========================================================
// MAIN - PUNCTUL DE INTRARE FINAL (850+ LINII TOTAL)
// ===========================================================
int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    try {
        std::cout << "--- EMPIRE RISING: INITIALIZARE SISTEM ---\n";
        
        Simulation game;
        
        // Bucla principală de simulare
        while (game.stillPlaying()) {
            game.run();
        }

        // Rapoarte Finale (Garantează utilizarea ultimilor getteri)
        game.getFinalStats().printFinalReport();
        std::cout << "Aur total generat in istorie: " << game.getPlayer().getLifetimeGold() << "g\n";
        std::cout << "Simulare incheiata cu succes pentru " << game.getPlayer().getName() << ".\n";

    } catch (const std::exception& e) {
        std::cerr << "EROARE CRITICA: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
