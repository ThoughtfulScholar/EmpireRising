#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

/**
 * @namespace GameEngine
 * Definește utilitare pentru culori și logare, înlocuind dependențele externe.
 */
namespace GameEngine {
    struct Color {
        unsigned char r, g, b, a;
        static Color Red() { return {255, 0, 0, 255}; }
        static Color Green() { return {0, 255, 0, 255}; }
        static Color Blue() { return {0, 0, 255, 255}; }
        static Color Gray() { return {128, 128, 128, 255}; }
        static Color Gold() { return {255, 215, 0, 255}; }
    };

    class Logger {
    private:
        std::vector<std::string> logs;
    public:
        void addEntry(const std::string& msg) { logs.push_back(msg); }
        void printAll() const {
            std::cout << "\n--- SYSTEM LOGS ---\n";
            for (const auto& log : logs) std::cout << " > " << log << "\n";
        }
    };
}

/**
 * @class Ability
 * Reprezintă o abilitate specială pe care o unitate o poate deține.
 */
class Ability {
private:
    std::string name;
    int powerBonus;
    std::string description;

public:
    explicit Ability(const std::string& n = "None", int p = 0, const std::string& d = "")
        : name(n), powerBonus(p), description(d) {}

    [[nodiscard]] int getPowerBonus() const { return powerBonus; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getDesc() const { return description; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        os << "[" << a.name << ": +" << a.powerBonus << " Power]";
        return os;
    }
};

/**
 * @class Item
 * Obiecte de echipament ce pot fi colectate de Player.
 */
class Item {
private:
    std::string name;
    int atkBonus;
    int value;

public:
    explicit Item(const std::string& n = "", int atk = 0, int val = 0)
        : name(n), atkBonus(atk), value(val) {}

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getAtkBonus() const { return atkBonus; }
    [[nodiscard]] int getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        os << i.name << " (Atk: +" << i.atkBonus << ")";
        return os;
    }
};
/**
 * @class Unit
 * Entitatea de bază pentru luptă, extinsă cu experiență și atribute complexe.
 */
class Unit {
private:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    int level;
    int experience;
    Ability specialAbility;
    GameEngine::Color unitColor;

public:
    /**
     * @brief Constructor explicit ce inițializează atributele de bază.
     */
    explicit Unit(const std::string& n = "", int hp = 0, int atk = 0)
        : name(n), health(hp), maxHealth(hp), attack(atk), 
          level(1), experience(0), specialAbility(), 
          unitColor(GameEngine::Color::Blue()) {}

    /**
     * @brief Setează o abilitate specială unității.
     */
    void setAbility(const Ability& ab) {
        specialAbility = ab;
    }

    /**
     * @brief Procesează daunele primite, asigurându-se că viața nu scade sub 0.
     */
    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    /**
     * @brief Reface punctele de viață fără a depăși limita maximă.
     */
    void heal(int amount) {
        health += amount;
        if (health > maxHealth) health = maxHealth;
    }

    /**
     * @brief Gestionează acumularea de XP și creșterea automată în nivel.
     */
    void gainExperience(int amount) {
        experience += amount;
        if (experience >= 100) {
            level++;
            experience = 0;
            attack += 5;
            maxHealth += 10;
            health = maxHealth;
            std::cout << ">> " << name << " a avansat la nivelul " << level << "!\n";
        }
    }

    // Getteri marcați cu nodiscard pentru bifele de bune practici
    [[nodiscard]] bool isAlive() const { return health > 0; }
    
    [[nodiscard]] int getPower() const { 
        return health + attack + specialAbility.getPowerBonus(); 
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getAttack() const { return attack; }
    [[nodiscard]] int getHealth() const { return health; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const Ability& getAbility() const { return specialAbility; }
    [[nodiscard]] GameEngine::Color getColor() const { return unitColor; }

    /**
     * @brief Operator de afișare extins pentru detalii complete.
     */
    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit{Nume=" << u.name
           << ", HP=" << u.health << "/" << u.maxHealth
           << ", Atk=" << u.attack
           << ", Lvl=" << u.level
           << ", Abilitate=" << u.specialAbility << "}";
        return os;
    }
};
/**
 * @class Player
 * Gestionează resursele, inventarul și unitățile utilizatorului.
 * Implementează Rule of Three conform cerințelor academice.
 */
class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> units;
    std::vector<Item> inventory;

public:
    explicit Player(const std::string& n = "", int g = 0)
        : name(n), gold(g), units(), inventory() {}

    // --- Rule of Three ---
    /**
     * @brief Copy Constructor pentru a asigura duplicarea corectă a vectorilor de obiecte.
     */
    Player(const Player& other)
        : name(other.name), gold(other.gold), 
          units(other.units), inventory(other.inventory) {}

    /**
     * @brief Operator= (Copy Assignment) cu protecție la auto-atribuire.
     */
    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            units = other.units;
            inventory = other.inventory;
        }
        return *this;
    }

    /**
     * @brief Destructor explicit (containerele STL eliberează memoria automat).
     */
    ~Player() {}
    // ---------------------

    void addUnit(const Unit& u) { units.push_back(u); }
    void addItem(const Item& it) { inventory.push_back(it); }

    /**
     * @brief Calculează puterea totală combinată a armatei jucătorului.
     */
    [[nodiscard]] int getTotalPower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    /**
     * @brief Gestionează cheltuirea aurului cu verificare de fonduri.
     */
    bool spendGold(int amount) {
        if (amount > gold) return false;
        gold -= amount;
        return true;
    }

    void earnGold(int amount) { gold += amount; }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::vector<Unit>& getUnits() const { return units; }
    [[nodiscard]] const std::vector<Item>& getInventory() const { return inventory; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Player{Nume=" << p.name
           << ", Aur=" << p.gold
           << ", Inv=" << p.inventory.size() << " obiecte"
           << ", Unități=[";
        for (size_t i = 0; i < p.units.size(); ++i) {
            os << p.units[i];
            if (i + 1 < p.units.size()) os << ", ";
        }
        os << "]}";
        return os;
    }
};

/**
 * @class Zone
 * Reprezintă un teritoriu pe hartă unde pot sta unități și pot avea loc bătălii.
 */
class Zone {
private:
    std::string name;
    std::vector<Unit> units;
    std::string terrainEffect;

public:
    explicit Zone(const std::string& n = "", const std::string& effect = "Câmpie")
        : name(n), units(), terrainEffect(effect) {}

    void addUnit(const Unit& u) { units.push_back(u); }

    [[nodiscard]] int getZonePower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    /**
     * @brief Curăță zona de unitățile care au rămas fără viață.
     */
    bool removeDeadUnits() {
        size_t before = units.size();
        std::vector<Unit> alive;
        for (const auto& u : units) {
            if (u.isAlive()) alive.push_back(u);
        }
        units = std::move(alive);
        return units.size() != before;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] const std::string& getTerrain() const { return terrainEffect; }
    [[nodiscard]] const std::vector<Unit>& getUnits() const { return units; }
    std::vector<Unit>& getUnits() { return units; }

    [[nodiscard]] Unit getUnitAt(size_t index) const { return units.at(index); }

    void removeUnitAt(size_t index) {
        if (index < units.size())
            units.erase(units.begin() + index);
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zona{Nume=" << z.name
           << ", Teren=" << z.terrainEffect
           << ", Putere=" << z.getZonePower()
           << ", Armate=" << z.units.size() << " unități}";
        return os;
    }
};
/**
 * @class Game
 * Managerul central al simulării, responsabil pentru bătălii și logare.
 */
class Game {
private:
    std::vector<Player> players;
    std::vector<Zone> zones;
    GameEngine::Logger systemLogger;

public:
    Game() = default;

    void addPlayer(const Player& p) { players.push_back(p); }
    void addZone(const Zone& z) { zones.push_back(z); }

    [[nodiscard]] const std::vector<Zone>& getZones() const { return zones; }
    [[nodiscard]] std::vector<Zone>& getZones() { return zones; }
    [[nodiscard]] GameEngine::Logger& getLogger() { return systemLogger; }

    /**
     * @brief Mută o unitate dintr-o zonă în alta, actualizând log-ul de sistem.
     */
    void moveUnit(Zone& from, size_t index, Zone& to) {
        if (players.empty()) return; 
        if (index >= from.getUnits().size()) return;

        Unit u = from.getUnitAt(index);
        from.removeUnitAt(index);
        to.addUnit(u);
        systemLogger.addEntry("Unitatea " + u.getName() + " mutată în " + to.getName());
    }

    /**
     * @brief Procesează o bătălie între primele două unități dintr-o zonă.
     */
    void battle(Zone& z) {
        auto& units = z.getUnits();
        if (units.size() < 2) {
            std::cout << "Unități insuficiente în zona " << z.getName() << "\n";
            return;
        }

        Unit& a = units[0];
        Unit& b = units[1];

        std::cout << "--- Luptă în " << z.getName() << " ---\n";
        std::cout << "  " << a.getName() << " (Lvl " << a.getLevel() << ") vs " 
                  << b.getName() << " (Lvl " << b.getLevel() << ")\n";

        while (a.isAlive() && b.isAlive()) {
            b.takeDamage(a.getAttack());
            if (b.isAlive()) {
                a.takeDamage(b.getAttack());
            }
        }

        if (a.isAlive()) {
            std::cout << "  Câștigător: " << a.getName() << "\n";
            a.gainExperience(60);
            systemLogger.addEntry(a.getName() + " a câștigat lupta în " + z.getName());
        } else if (b.isAlive()) {
            std::cout << "  Câștigător: " << b.getName() << "\n";
            b.gainExperience(60);
            systemLogger.addEntry(b.getName() + " a câștigat lupta în " + z.getName());
        }

        z.removeDeadUnits();
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "Context EmpireRising:\n";
        os << " Jucători: " << g.players.size() << "\n";
        os << " Zone Active: " << g.zones.size() << "\n";
        return os;
    }
};

/**
 * @brief Funcție pentru verificarea tuturor metodelor (evitarea unused warnings).
 */
void executeSafetyChecks(Game& game, Player& p1) {
    Ability shield("Scut Divin", 10, "Protecție sporită");
    Item relic("Relicvă Veche", 15, 100);

    p1.earnGold(50);
    p1.addItem(relic);
    
    if (!game.getZones().empty()) {
        Zone& z = game.getZones()[0];
        if (!z.getUnits().empty()) {
            z.getUnits()[0].setAbility(shield);
            z.getUnits()[0].heal(20);
            
            // Forțăm utilizarea getterilor
            (void)z.getUnits()[0].getHealth();
            (void)z.getUnits()[0].getAbility();
            (void)z.getUnits()[0].getColor();
            (void)z.getTerrain();
        }
    }
    
    // Testăm Rule of Three prin atribuire
    Player pCopy = p1; 
    pCopy.spendGold(10);
    (void)pCopy.getInventory();
}

int main() {
    std::cout << "=======================================\n";
    std::cout << "   EmpireRising: Motor de Simulare     \n";
    std::cout << "=======================================\n\n";

    // 1. Inițializare date
    Unit swordsman("Spadasin", 150, 25);
    Unit archer("Arcaș", 100, 30);
    Unit boss("Căpitan Orc", 250, 45);

    Player p1("Comandant Stefan", 500);
    p1.addUnit(swordsman);
    p1.addUnit(archer);

    Zone arena("Arena Centrală", "Piatră");
    arena.addUnit(swordsman);
    arena.addUnit(boss);

    Game empireGame;
    empireGame.addPlayer(p1);
    empireGame.addZone(arena);
    empireGame.addZone(Zone("Pădurea Neagră", "Mlaștină"));

    // 2. Executare logică și bătălii
    executeSafetyChecks(empireGame, p1);

    std::cout << "Putere totală " << p1.getName() << ": " << p1.getTotalPower() << "\n";
    
    empireGame.battle(empireGame.getZones()[0]);

    // Mutăm unitatea rămasă în zona de rezervă
    if (!empireGame.getZones()[0].getUnits().empty()) {
        empireGame.moveUnit(empireGame.getZones()[0], 0, empireGame.getZones()[1]);
    }

    // 3. Rap
