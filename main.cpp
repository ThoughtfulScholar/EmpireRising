#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>
#include <map>
#include <stdexcept>
#include "raylib-cpp.hpp"

// ==========================================================
// 1. ENUMS ȘI CONSTANTE GLOBALE
// ==========================================================
enum class UnitType { INFANTERIE, ARCASI, CAVALERIE, GARDA, EROU };
enum class TerrainType { PLAIN, FOREST, MOUNTAIN, WATER, CITY_TILE };
enum class GameState { LOGIN, SIMULATION, VICTORIE, DEFEAT };

// ==========================================================
// 2. IERARHIA DE EXCEPȚII (Cerință Tema 2)
// ==========================================================
class EmpireException : public std::runtime_error {
public:
    explicit EmpireException(const std::string& msg) : std::runtime_error(msg) {}
};

class InsufficientGoldException : public EmpireException {
public:
    InsufficientGoldException() : EmpireException("Tezaur insuficient!") {}
};

class PopulationLimitException : public EmpireException {
public:
    explicit PopulationLimitException(const std::string& msg) : EmpireException(msg) {}
};

class CombatException : public EmpireException {
public:
    explicit CombatException(const std::string& msg) : EmpireException(msg) {}
};

class InvalidMovementException : public EmpireException {
public:
    explicit InvalidMovementException(const std::string& msg) : EmpireException(msg) {}
};
// ==========================================================
// 3. UTILS: RANDOM GENERATOR & LOGGER (Statice - Tema 2)
// ==========================================================
namespace GameEngine {
    class RandomGen {
    private:
        static std::mt19937& getEngine() {
            static std::random_device rd;
            static std::mt19937 engine(rd());
            return engine;
        }
    public:
        static int GetInt(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(getEngine());
        }
        static float GetFloat(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(getEngine());
        }
    };

    class Logger {
    private:
        std::vector<std::string> messages;
        static int totalLogEntries; // Atribut static (Cerință Tema 2)

    public:
        void add(const std::string& msg) {
            messages.push_back(msg);
            totalLogEntries++;
            if (messages.size() > 10) { // Menținem doar ultimele 10 log-uri pentru vizibilitate
                messages.erase(messages.begin());
            }
        }

        void logError(const std::exception& e) {
            add("![EROARE]: " + std::string(e.what()));
        }

        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }

        // Metodă statică (Cerință Tema 2)
        static int getTotalLogs() { return totalLogEntries; }
    };
}

// Inițializare atribut static
int GameEngine::Logger::totalLogEntries = 0;
// ==========================================================
// 4. ABILITY (Compunere - Cerință Tema 1)
// ==========================================================
class Ability {
private:
    std::string name;
    float activationChance;
    int bonusDamage;

public:
    explicit Ability(std::string n = "Atac Standard", float chance = 0.0f, int bonus = 0)
        : name(std::move(n)), activationChance(chance), bonusDamage(bonus) {}

    [[nodiscard]] int trigger() const {
        if (GameEngine::RandomGen::GetFloat(0.0f, 1.0f) <= activationChance) return bonusDamage;
        return 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
};

// ==========================================================
// 5. UNIT (Clasă de Bază Polimorfică - Cerințe Tema 2)
// ==========================================================
// ==========================================================
// CLASA UNIT - BAZA POLIMORFISMULUI (PARTEA 1)
// ==========================================================
class Unit {
protected:
    std::string name;
    int hp;
    int maxHp;
    int atk;
    int upkeepCost;
    int level;
    int xp;

    // Membru static pentru contorizarea globala (Cerinta Tema 2)
    static int totalUnitsCreated;

public:
    // Constructor de initializare (Fara parametrul def)
    Unit(std::string n, int h, int a, int u)
        : name(std::move(n)), hp(h), maxHp(h), atk(a),
          upkeepCost(u), level(1), xp(0) {
        totalUnitsCreated++;
    }

    // Rule of Three: Destructor virtual obligatoriu
    virtual ~Unit() = default;

    // Constructor de copiere (Prototype Pattern)
    Unit(const Unit& other)
        : name(other.name), hp(other.hp), maxHp(other.maxHp),
          atk(other.atk), upkeepCost(other.upkeepCost),
          level(other.level), xp(other.xp) {
        totalUnitsCreated++;
    }

    // Operator de atribuire
    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            hp = other.hp;
            maxHp = other.maxHp;
            atk = other.atk;
            upkeepCost = other.upkeepCost;
            level = other.level;
            xp = other.xp;
        }
        return *this;
    }

    // Metoda pur virtuala pentru clonare
    virtual std::unique_ptr<Unit> clone() const = 0;

    // --- LOGICA DE LUPTA SI STATISTICI ---

    // Atacul creste cu nivelul
    [[nodiscard]] virtual int calculateTotalAttack() const {
        return atk + (level * 5);
    }

    // Damage-ul scade direct din HP
    virtual void takeDamage(int rawDamage) {
        hp -= rawDamage;
        if (hp < 0) hp = 0;
    }

    [[nodiscard]] bool isAlive() const {
        return hp > 0;
    }

    // Necesar pentru compatibilitatea cu motorul de joc (Zone.cpp)
    void playAttackSound() const {}

    // --- EXPERIENTA SI NIVEL ---

    void gainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp = 0;
            maxHp += 30; // Bonus viata
            hp = maxHp;  // Vindecare completa la level up
            atk += 10;   // Bonus atac
        }
    }

    // --- GETTERI ---
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getAtk() const { return atk; }
    [[nodiscard]] int getUpkeep() const { return upkeepCost; }
    [[nodiscard]] int getLevel() const { return level; }

    // --- METODE STATICE ---
    static int getTotalUnits() {
        return totalUnitsCreated;
    }

    // --- OPERATORI ---
    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << u.name << " [Lvl " << u.level << " | HP: " << u.hp << "/" << u.maxHp
           << " | ATK: " << u.atk << "]";
        return os;
    }
};

// Initializarea membrului static (Obligatoriu sub clasa)
int Unit::totalUnitsCreated = 0;
// ==========================================================
// 6. CLASE DERIVATE (Ierarhie Polimorfică - Tema 2)
// ==========================================================

class Infantry : public Unit {
public:
    // Trimitem: nume, hp, atac, upkeep
    Infantry() : Unit("Infanterie", 300, 45, 25) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Infantry>(*this); }
};

class Archer : public Unit {
public:
    Archer() : Unit("Arcas", 180, 70, 30) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Archer>(*this); }
};

class Cavalry : public Unit {
public:
    Cavalry() : Unit("Cavalerie", 350, 60, 50) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Cavalry>(*this); }
};

class GarrisonGuard : public Unit {
public:
    GarrisonGuard(std::string n) : Unit(n, 250, 40, 15) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<GarrisonGuard>(*this); }
};

class Hero : public Unit {
public:
    // Am scos 'int d' din lista de parametri
    Hero(std::string n, int h, int a, int u) : Unit(n, h, a, u) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Hero>(*this); }
    void inspire() {}
};
// ==========================================================
// 7. UNIT FACTORY (Cerință Tema 2 - Creare Polimorfică)
// ==========================================================
struct UnitStats {
    std::string name;
    int hp;
    int atk;
    int cost;
    int upkeep; // Al 5-lea element
};

static std::map<UnitType, UnitStats> GameData = {
    // { TIP, { NUME, HP, ATK, COST, UPKEEP } }
    {UnitType::INFANTERIE, {"Infanterie", 300, 45, 150, 25}},
    {UnitType::ARCASI,     {"Arcas",      180, 70, 180, 30}},
    {UnitType::CAVALERIE,  {"Cavalerie",  350, 60, 300, 50}},
    {UnitType::GARDA,      {"Garda",      250, 40, 100, 15}},
    {UnitType::EROU,       {"Erou",       600, 90, 1000, 100}}
};

struct UnitFactory {
    static std::unique_ptr<Unit> CreateUnit(UnitType type, const std::string& customName = "") {
        auto s = GameData[type];
        std::string fName = customName.empty() ? s.name : customName;

        switch (type) {
            case UnitType::INFANTERIE: return std::make_unique<Infantry>();
            case UnitType::ARCASI:     return std::make_unique<Archer>();
            case UnitType::CAVALERIE:  return std::make_unique<Cavalry>();
            case UnitType::GARDA:      return std::make_unique<GarrisonGuard>(fName);
            case UnitType::EROU:       return std::make_unique<Hero>(fName, s.hp, s.atk, s.upkeep);
            default: return nullptr;
        }
    }
};

// ==========================================================
// 8. ARMY MANAGER (Rule of Three & Gestiune Resurse - Tema 2)
// ==========================================================
class ArmyManager {
private:
    std::vector<std::unique_ptr<Unit>> units;

public:
    ArmyManager() = default;

    // --- RULE OF THREE: COPY CONSTRUCTOR ---
    ArmyManager(const ArmyManager& other) {
        for (const auto& u : other.units) {
            if (u) units.push_back(u->clone());
        }
    }

    // --- RULE OF THREE: COPY ASSIGNMENT (Copy and Swap) ---
    ArmyManager& operator=(ArmyManager other) {
        std::swap(units, other.units);
        return *this;
    }

    // Destructorul este automat (unique_ptr se ocupă de cleanup)
    ~ArmyManager() = default;

    void addUnit(std::unique_ptr<Unit> u) {
        if (u) units.push_back(std::move(u));
    }

    // Metodă pentru extragerea ultimei unități (pentru retragere din garnizoană)
    std::unique_ptr<Unit> popUnit() {
        if (units.empty()) return nullptr;
        std::unique_ptr<Unit> u = std::move(units.back());
        units.pop_back();
        return u;
    }

    void removeDeadUnits() {
        std::erase_if(units, [](const auto& u) { return !u->isAlive(); });
    }

    [[nodiscard]] bool isEmpty() const { return units.empty(); }

    [[nodiscard]] Unit* getFrontUnit() {
        return units.empty() ? nullptr : units.front().get();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getUnits() const {
        return units;
    }

    // Cerință Tema 2: dynamic_cast pentru a găsi și inspira eroii
    void inspireHeroes() {
        for (auto& u : units) {
            if (auto* hero = dynamic_cast<Hero*>(u.get())) {
                hero->inspire();
            }
        }
    }

    [[nodiscard]] int calculateTotalUpkeep() const {
        int total = 0;
        for (const auto& u : units) total += u->getUpkeep();
        return total;
    }

    // Metodă pentru numărarea pe tipuri (pentru UI)
    [[nodiscard]] std::map<std::string, int> getUnitCounts() const {
        std::map<std::string, int> counts;
        for (const auto& u : units) counts[u->getName()]++;
        return counts;
    }
    std::vector<std::unique_ptr<Unit>>& getUnits() { return units; }
};
// ==========================================================
// 9. WORLD MAP & TILE (Gestiune relief și drumuri)
// ==========================================================

class Tile {
private:
    TerrainType type;
    bool walkable;
public:
    explicit Tile(TerrainType t = TerrainType::PLAIN) : type(t) {
        walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
    }
    [[nodiscard]] TerrainType getType() const { return type; }
    [[nodiscard]] bool isWalkable() const { return walkable; }
    void setType(TerrainType t) {
        type = t;
        walkable = (t != TerrainType::MOUNTAIN && t != TerrainType::WATER);
    }
};

class WorldMap {
private:
    int width, height;
    std::vector<std::vector<Tile>> grid;

public:
    WorldMap(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Tile>(width, Tile(TerrainType::PLAIN)));
    }

    // Algoritm care sapă un drum garantat (Cerința ta)
    void createPath(int x1, int y1, int x2, int y2) {
        int curX = x1;
        int curY = y1;
        while (curX != x2 || curY != y2) {
            if (curX < x2) curX++;
            else if (curX > x2) curX--;
            else if (curY < y2) curY++;
            else if (curY > y2) curY--;

            // Transformăm orice obstacol în drum practicabil
            if (grid[curY][curX].getType() != TerrainType::CITY_TILE) {
                grid[curY][curX].setType(TerrainType::PLAIN);
            }
        }
    }

    void setTile(int x, int y, TerrainType t) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x].setType(t);
        }
    }

    [[nodiscard]] bool isValidMove(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        return grid[y][x].isWalkable();
    }

    void draw(int offX, int offY) const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Specificăm explicit tipul pentru a evita ambiguitatea
                raylib::Color tileColor;

                switch (grid[y][x].getType()) {
                    case TerrainType::FOREST:
                        tileColor = raylib::Color(34, 139, 34, 255); break;
                    case TerrainType::MOUNTAIN:
                        tileColor = raylib::Color(105, 105, 105, 255); break;
                    case TerrainType::WATER:
                        tileColor = raylib::Color(0, 191, 255, 255); break;
                    case TerrainType::CITY_TILE:
                        tileColor = raylib::Color(80, 80, 90, 255); break;
                    default:
                        tileColor = raylib::Color(100, 150, 70, 255); break;
                }
                DrawRectangle(offX + x * 40, offY + y * 40, 39, 39, tileColor);
            }
        }
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};
// ==========================================================
// CLASA CITY - REPARATA PENTRU LOGGER SI ECONOMIE
// ==========================================================
#include <vector>
#include <memory>
#include <string>
#include <map>

class City {
private:
    std::string name;
    int posX, posY;
    int population;
    int cityLevel;
    bool occupied;

    // TEMA 2: Gestiune automată a memoriei pentru unitățile de garnizoană
    std::vector<std::unique_ptr<Unit>> garrison;

public:
    // Constructor principal
    City(std::string n, int x, int y, int pop = 150)
        : name(std::move(n)), posX(x), posY(y), population(pop), cityLevel(1), occupied(false) {}

    // RULE OF THREE: Necesar pentru a copia corect unique_ptr prin clonare (Tema 2)
    City(const City& other)
        : name(other.name), posX(other.posX), posY(other.posY),
          population(other.population), cityLevel(other.cityLevel), occupied(other.occupied) {
        for (const auto& u : other.garrison) {
            if (u) garrison.push_back(u->clone());
        }
    }

    City& operator=(const City& other) {
        if (this != &other) {
            name = other.name; posX = other.posX; posY = other.posY;
            population = other.population; cityLevel = other.cityLevel;
            occupied = other.occupied;
            garrison.clear();
            for (const auto& u : other.garrison) {
                if (u) garrison.push_back(u->clone());
            }
        }
        return *this;
    }

    // --- MECANICI DE EVOLUȚIE (TEMA 1) ---

    void upgrade() {
        cityLevel++;
        population += 100; // Bonus de expansiune
    }

    void growPopulation() {
        if (occupied) {
            // Populația crește în funcție de stabilitate și nivel
            population += (20 * cityLevel);
        }
    }

    // --- LOGICA MATEMATICĂ DE REBELIUNE (1 unitate la 100 oameni) ---

    template<typename T>
    void checkRebellion(T& logger) {
        if (!occupied) return;

        // Formula cerută: 1 unitate la 100 de locuitori
        int unitsNeeded = population / 100;
        if (unitsNeeded < 1) unitsNeeded = 1; // Minim o unitate pentru control

        int currentGarrisonCount = (int)garrison.size();

        // Dacă garnizoana este insuficientă, calculăm riscul
        if (currentGarrisonCount < unitsNeeded) {
            // Exemplu: Pop 800 -> needed 8. Ai 1 unitate. Șansă: (8-1)/8 = 87.5%
            float failChance = (float)(unitsNeeded - currentGarrisonCount) / unitsNeeded;
            float roll = (float)GetRandomValue(0, 100) / 100.0f;

            if (roll < failChance) {
                occupied = false;
                garrison.clear(); // Garnizoana este eliminată de revoltați (Tema 2 - memorie eliberată)
                logger.add("RASCOALA în " + name + "! Populatia a preluat controlul (Risc: " + std::to_string((int)(failChance*100)) + "%)");
            }
        }
    }

    // --- ECONOMIE (TAXE ȘI SALARII) ---

    [[nodiscard]] int collectTaxes() const {
        if (!occupied) return 0;
        // Taxe mult mai mari conform cerinței (Populația / 2 * Nivel)
        return (population / 2) * cityLevel;
    }

    [[nodiscard]] int getGarrisonUpkeep() const {
        int total = 0;
        for (const auto& u : garrison) {
            if (u) total += u->getUpkeep();
        }
        return total;
    }

    // --- GESTIONARE TRUPE ---

    void addUnitToGarrison(std::unique_ptr<Unit> u) {
        if (u) {
            garrison.push_back(std::move(u));
            occupied = true; // Dacă pui trupe, orașul devine ocupat/al tău
        }
    }

    std::unique_ptr<Unit> extractUnit() {
        if (garrison.empty()) return nullptr;
        auto u = std::move(garrison.back());
        garrison.pop_back();
        return u;
    }

    void clearGarrison() {
        garrison.clear();
    }

    // --- GETTERI ---

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
    [[nodiscard]] int getPopulation() const { return population; }
    [[nodiscard]] int getCityLevel() const { return cityLevel; }
    [[nodiscard]] int getUpgradeCost() const { return cityLevel * 500; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    void setOccupied(bool state) { occupied = state; }

    std::vector<std::unique_ptr<Unit>>& getGarrison() { return garrison; }

    [[nodiscard]] std::map<std::string, int> getGarrisonCounts() const {
        std::map<std::string, int> counts;
        for (const auto& u : garrison) {
            if (u) counts[u->getName()]++;
        }
        return counts;
    }
};
// ==========================================================
// 11. ENEMY ARMY & ZONE (Logica de Asediu)
// ==========================================

class EnemyArmy {
private:
    int posX, posY;
    ArmyManager troops;
    std::string identifier;

public:
    EnemyArmy(int x, int y, std::string id)
        : posX(x), posY(y), identifier(std::move(id)) {
        // Generăm trupe inamice aleatorii
        int numUnits = GameEngine::RandomGen::GetInt(1, 3);
        for(int i = 0; i < numUnits; ++i) {
            troops.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Jafuitor"));
        }
    }

    [[nodiscard]] int getX() const { return posX; }
    [[nodiscard]] int getY() const { return posY; }
    [[nodiscard]] bool isDefeated() const { return troops.isEmpty(); }
    [[nodiscard]] ArmyManager& getTroops() { return troops; }

    void draw(int offX, int offY) const {
        float px = (float)(offX + posX * 40 + 20);
        float py = (float)(offY + posY * 40 + 20);
        DrawPoly({px, py}, 4, 15, 0, RED); // Romb roșu pentru inamici mobili
        DrawPolyLines({px, py}, 4, 16, 0, WHITE);
    }
};

class Zone {
private:
    std::string zoneName;
    City localCity;
    ArmyManager enemyGarrison; // Inamicii care păzesc orașul înainte de cucerire
    raylib::Color mapTint;

public:
    Zone(std::string name, City city, raylib::Color tint)
        : zoneName(std::move(name)), localCity(std::move(city)), mapTint(tint) {

        // Dacă nu este capitala, îi punem o gardă inamică inițială
        if (zoneName != "Provincia Natala") {
            int guards = GameEngine::RandomGen::GetInt(2, 3);
            for(int i = 0; i < guards; ++i) {
                enemyGarrison.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Garda Inamica"));
            }
        }
    }

    // Metoda de luptă (NVI - Non-Virtual Interface în esență prin apeluri polimorfice)
    [[nodiscard]] std::string executeBattleRound(ArmyManager& playerArmy) {
        if (localCity.isOccupied()) return "Orasul este deja sub controlul tau.";
        if (playerArmy.isEmpty()) throw CombatException("Nu ai trupe pentru asediu!");

        Unit* pUnit = playerArmy.getFrontUnit();
        Unit* eUnit = enemyGarrison.getFrontUnit();

        // 1. Atacul Jucătorului
        int pAtk = pUnit->calculateTotalAttack();
        eUnit->takeDamage(pAtk);
        pUnit->playAttackSound();

        std::string log = pUnit->getName() + " loveste cu " + std::to_string(pAtk) + ". ";

        if (!eUnit->isAlive()) {
            enemyGarrison.removeDeadUnits();
            pUnit->gainXP(50);
            if (enemyGarrison.isEmpty()) {
                localCity.setOccupied(true);
                return log + "VICTORIE! Cetatea a fost cucerita.";
            }
            return log + "Inamic rapus!";
        }

        // 2. Riposta Inamicului
        int eAtk = eUnit->calculateTotalAttack();
        pUnit->takeDamage(eAtk);
        log += "Inamicul riposteaza: -" + std::to_string(eAtk) + " HP.";

        if (!pUnit->isAlive()) {
            playerArmy.removeDeadUnits();
            return log + " Unitatea ta a cazut!";
        }

        return log;
    }

    // Getters
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const City& getCity() const { return localCity; }
    [[nodiscard]] ArmyManager& getEnemyGarrison() { return enemyGarrison; }
};
// ==========================================================
// 12. PLAYER (Comandantul)
// ==========================================================
class Player {
private:
    std::string commanderName;
    int gold;
    ArmyManager army;
    int posX, posY;

public:
    explicit Player(std::string name = "Comandant", int startingGold = 1200)
        : commanderName(std::move(name)), gold(startingGold), posX(2), posY(2) {}

    // --- LOGICA LIMITĂ POPULAȚIE ---
    [[nodiscard]] int getUnitLimit(const std::vector<Zone>& regions) const {
        int limit = 4; // Limită de bază (corturile de campanie)
        for (const auto& r : regions) {
            if (r.getCity().isOccupied()) {
                // Fiecare oraș cucerit oferă +3 sloturi de unități
                limit += 3;
            }
        }
        return limit;
    }

    void recruit(std::unique_ptr<Unit> u, int cost, int maxLimit) {
        if (static_cast<int>(army.getUnits().size()) >= maxLimit) {
            throw PopulationLimitException("Limita de populatie atinsa! Cucereste orase pentru mai multe sloturi.");
        }
        if (gold >= cost) {
            gold -= cost;
            army.addUnit(std::move(u));
        } else {
            throw InsufficientGoldException();
        }
    }

    void move(int dx, int dy, const WorldMap& worldMap) {
        int newX = posX + dx;
        int newY = posY + dy;
        if (!worldMap.isValidMove(newX, newY)) {
            throw InvalidMovementException("Teren impracticabil!");
        }
        posX = newX;
        posY = newY;
    }

    void earnGold(int amount) { gold += amount; }
    void removeGold(int amount) { gold -= amount; }
    // Getters
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] ArmyManager& getArmy() { return army; }
    [[nodiscard]] const ArmyManager& getArmy() const { return army; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
};

class Enemy {
private:
    int x, y;
    int strength;
public:
    Enemy(int x, int y, int strength) : x(x), y(y), strength(strength) {}
    int getX() const { return x; }
    int getY() const { return y; }
    int getStrength() const { return strength; }
};

class GarrisonUnit : public Unit {
public:
    GarrisonUnit() : Unit("Garnizoana", 200, 30, 10) {} // Nume, HP, Atac, Upkeep mic
    std::unique_ptr<Unit> clone() const override { return std::make_unique<GarrisonUnit>(*this); }
};

// ==========================================================
// 13. WORLD CLOCK (Sistemul de Timp)
// ==========================================================
class WorldClock {
private:
    int currentDay;
    static float globalTaxModifier; // Atribut static (Tema 2)

public:
    explicit WorldClock() : currentDay(1) {}

    void nextDay() { currentDay++; }
    [[nodiscard]] int getDay() const { return currentDay; }

    static void SetGlobalTaxRate(float rate) { globalTaxModifier = rate; }
    static float GetGlobalTaxRate() { return globalTaxModifier; }
};

float WorldClock::globalTaxModifier = 1.0f;
// ==========================================================
// 14. SIMULATION (Gestiunea Centrală a Jocului)
// ==========================================================
class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    Player player;
    WorldMap worldMap;
    WorldClock clock;
    GameEngine::Logger logger;

    std::vector<Zone> regions;
    std::vector<EnemyArmy> roamingEnemies;

    int activeRegionIdx = 0;
    bool showRecruitment = false;
    bool gameOver = false;

    static constexpr int MAX_DAYS = 50;
    static constexpr int SIDEBAR_WIDTH = 400;

    // --- LOGICA ECONOMICĂ (Cerința Net Gold) ---
    [[nodiscard]] int calculateTotalIncome() const {
        int total = 0;
        for (const auto& r : regions) {
            total += r.getCity().collectTaxes();
        }
        return static_cast<int>(total * WorldClock::GetGlobalTaxRate());
    }

    [[nodiscard]] int calculateTotalUpkeep() const {
        int upkeep = player.getArmy().calculateTotalUpkeep();
        for (const auto& r : regions) {
            upkeep += r.getCity().getGarrisonUpkeep();
        }
        return upkeep;
    }

    [[nodiscard]] int getNetGold() const {
        return calculateTotalIncome() - calculateTotalUpkeep();
    }

    // --- INIȚIALIZARE LUME (Drumuri Garantate) ---
    void initWorld() {
        // 1. Resetăm harta la câmpie (Plain) - deja făcut de constructor, dar e bine de știut

        // 2. Generăm relief natural variat
        for (int i = 0; i < 40; ++i) {
            int tx = GameEngine::RandomGen::GetInt(0, worldMap.getWidth() - 1);
            int ty = GameEngine::RandomGen::GetInt(0, worldMap.getHeight() - 1);

            int roll = GameEngine::RandomGen::GetInt(0, 100);
            if (roll < 15) worldMap.setTile(tx, ty, TerrainType::MOUNTAIN);
            else if (roll < 30) worldMap.setTile(tx, ty, TerrainType::FOREST);
            else if (roll < 40) worldMap.setTile(tx, ty, TerrainType::WATER);
        }

        // 3. Plasare Orașe cu Drumuri Garantate
        for (int i = 0; i < 6; ++i) {
            int cx = GameEngine::RandomGen::GetInt(2, 27);
            int cy = GameEngine::RandomGen::GetInt(2, 17);

            // Relief de oraș
            worldMap.setTile(cx, cy, TerrainType::CITY_TILE);
            // Drum garantat de la poziția jucătorului (2,2) la oraș
            worldMap.createPath(2, 2, cx, cy);

            City c((i == 0 ? "Capitala" : "Cetate Inamica " + std::to_string(i)), cx, cy);
            if (i == 0) {
                c.setOccupied(true);
                regions.emplace_back("Provincia Natala", std::move(c), GREEN);
            } else {
                regions.emplace_back("Tinut Inamic", std::move(c), RED);
            }
        }
    }

    // --- VERIFICARE STARE JOC ---
    void checkGameStatus() {
        bool allCaptured = std::all_of(regions.begin(), regions.end(), [](const Zone& z) {
            return z.getCity().isOccupied();
        });

        bool noCitiesLeft = std::none_of(regions.begin(), regions.end(), [](const Zone& z) {
            return z.getCity().isOccupied();
        });

        if (allCaptured) currentState = GameState::VICTORIE;
        else if (noCitiesLeft || clock.getDay() >= MAX_DAYS) currentState = GameState::DEFEAT;
    }

    void processNextDay() {
        // 0. Dacă jocul s-a terminat, nu mai procesăm nimic
        if (gameOver) return;

        // 1. Avansăm calendarul și logăm începutul zilei
        clock.nextDay();
        logger.add("--- ZIUA " + std::to_string(clock.getDay()) + " ---");

        int totalIncome = 0;
        int totalUpkeep = 0;
        int occupiedCitiesCount = 0;

        // 2. Calculăm Upkeep-ul Armatei Mobile (Jucătorul)
        // Presupunem că player.getArmy() returnează obiectul Army
        totalUpkeep += player.getArmy().calculateTotalUpkeep();

        // 3. Procesăm fiecare regiune/cetate de pe hartă
        for (auto& reg : regions) {
            City& city = reg.getCity();

            if (city.isOccupied()) {
                occupiedCitiesCount++;

                // A. Evoluție (Tema 1): Creștem populația orașului
                city.growPopulation();

                // B. Economie: Adunăm taxele și upkeep-ul garnizoanei specifice
                totalIncome += city.collectTaxes();
                totalUpkeep += city.getGarrisonUpkeep();

                // C. LOGICA MATEMATICĂ DE REBELIUNE (Cerința ta: 1 unitate la 100 oameni)
                int pop = city.getPopulation();
                int currentGarrison = (int)city.getGarrison().size();
                int unitsNeeded = pop / 100;
                if (unitsNeeded < 1) unitsNeeded = 1; // Minim o unitate necesară

                if (currentGarrison < unitsNeeded) {
                    // Formula ta: (8 - 1) / 8 = 7/8 șansă de eșec
                    float failChance = (float)(unitsNeeded - currentGarrison) / unitsNeeded;
                    float roll = (float)GetRandomValue(0, 100) / 100.0f;

                    if (roll < failChance) {
                        city.setOccupied(false);
                        city.clearGarrison(); // Memoria unique_ptr este eliberată automat (Tema 2)
                        logger.add("!!! REBELIUNE în " + city.getName() + " (Risc: " + std::to_string((int)(failChance*100)) + "%)");
                        occupiedCitiesCount--;
                    }
                }
            }
        }

        // 4. Aplicăm tranzacția financiară (PROFIT NET)
        int netProfit = totalIncome - totalUpkeep;
        player.earnGold(netProfit);

        logger.add(TextFormat("Finanțe: Taxe +%i | Salarii -%i | Net: %+i", totalIncome, totalUpkeep, netProfit));

        // 5. SPAWNATOR DE ARMATE INAMICE (Evenimente externe la fiecare 5 zile)
        if (clock.getDay() % 5 == 0) {
            // Șansă de atac asupra unei cetăți controlate
            if (occupiedCitiesCount > 0) {
                int targetIdx = GetRandomValue(0, regions.size() - 1);
                if (regions[targetIdx].getCity().isOccupied()) {
                    logger.add("ATAC INAMIC: O armată străină a asediat " + regions[targetIdx].getCity().getName() + "!");
                    // Dacă nu există garnizoană deloc, orașul cade instant
                    if (regions[targetIdx].getCity().getGarrison().empty()) {
                        regions[targetIdx].getCity().setOccupied(false);
                        logger.add("Cetatea a căzut fără luptă!");
                    } else {
                        // Luptă: Moare o unitate de garnizoană
                        regions[targetIdx].getCity().extractUnit();
                        logger.add("Garnizoana a respins atacul, dar a pierdut oameni.");
                    }
                }
            }
        }

        // 6. VERIFICARE CONDIȚII DE PIERDERE (GAME OVER)

        // A. Pierdere prin lipsă de teritoriu
        if (occupiedCitiesCount <= 0 && clock.getDay() > 1) {
            gameOver = true;
            logger.add("DEZASTRU: Nu mai controlezi nicio cetate! Poporul te-a uitat.");
        }

        // B. Pierdere prin faliment (Dacă rămâi fără aur și nu poți plăti trupele)
        if (player.getGold() < -500) {
            gameOver = true;
            logger.add("FALIMENT: Tezaurul este ruinat. Armata a dezertat în masă.");
        }
    }
    // --- LOGICA DE RECRUTARE (Meniul R) ---
    void handleRecruitment() {
        if (!showRecruitment) return; // Dacă meniul nu e vizibil, nu recrutăm

        int typeToRecruit = -1;
        if (IsKeyPressed(KEY_ONE)) typeToRecruit = (int)UnitType::INFANTERIE;
        if (IsKeyPressed(KEY_TWO)) typeToRecruit = (int)UnitType::ARCASI;
        if (IsKeyPressed(KEY_THREE)) typeToRecruit = (int)UnitType::CAVALERIE;
        if (IsKeyPressed(KEY_FOUR)) typeToRecruit = (int)UnitType::GARDA;

        if (typeToRecruit != -1) {
            UnitType t = static_cast<UnitType>(typeToRecruit);
            auto stats = GameData[t];
            try {
                int limit = player.getUnitLimit(regions);
                player.recruit(UnitFactory::CreateUnit(t, "Soldat"), stats.cost, limit);
                logger.add("Recrutat: Soldat (" + std::to_string(stats.cost) + " aur)");
            } catch (const EmpireException& e) {
                logger.logError(e);
            }
        }
    }

    void tryRecruit(UnitType type, const std::string& name, int cost) {
        try {
            int limit = player.getUnitLimit(regions);
            player.recruit(UnitFactory::CreateUnit(type, name), cost, limit);
            logger.add("Recrutare reusita: " + name);
        } catch (const EmpireException& e) { logger.logError(e); }
    }

    // --- LOGICA DE LUPTĂ ---
    void handleCombat() {
        auto [px, py] = player.getPos();

        // 1. Căutăm inamici mobili (roaming)
        auto it = std::find_if(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
            return std::abs(e.getX() - px) <= 1 && std::abs(e.getY() - py) <= 1 && !e.isDefeated();
        });

        if (it != roamingEnemies.end()) {
            try {
                Unit* pUnit = player.getArmy().getFrontUnit();
                Unit* eUnit = it->getTroops().getFrontUnit();
                if(!pUnit) throw CombatException("Armata ta este goala!");

                eUnit->takeDamage(pUnit->calculateTotalAttack());
                if (eUnit->isAlive()) pUnit->takeDamage(eUnit->calculateTotalAttack());

                it->getTroops().removeDeadUnits();
                player.getArmy().removeDeadUnits();
                logger.add("Lupta pe camp deschis!");
            } catch (const EmpireException& e) { logger.logError(e); }
            return;
        }

        // 2. Verificăm asediu oraș
        auto& sel = regions[activeRegionIdx];
        auto [cx, cy] = sel.getCity().getPos();
        if (px == cx && py == cy) {
            try {
                std::string result = sel.executeBattleRound(player.getArmy());
                logger.add(result);
            } catch (const EmpireException& e) { logger.logError(e); }
        }
    }

    void handleInput() {
        // 1. Deschidere/Închidere Meniu Recrutare
        if (IsKeyPressed(KEY_R)) {
            showRecruitment = !showRecruitment;
            return;
        }

        // 2. LOGICA CÂND MENIUL E DESCHIS (Recrutare activă)
        if (showRecruitment) {
            int typeIdx = -1;
            if (IsKeyPressed(KEY_ONE)) typeIdx = (int)UnitType::INFANTERIE;
            else if (IsKeyPressed(KEY_TWO)) typeIdx = (int)UnitType::ARCASI;
            else if (IsKeyPressed(KEY_THREE)) typeIdx = (int)UnitType::CAVALERIE;
            else if (IsKeyPressed(KEY_FOUR)) typeIdx = (int)UnitType::GARDA;

            if (typeIdx != -1) {
                UnitType t = static_cast<UnitType>(typeIdx);
                auto stats = GameData[t];
                try {
                    int limit = player.getUnitLimit(regions);
                    player.recruit(UnitFactory::CreateUnit(t, stats.name), stats.cost, limit);
                    logger.add("Recrutat: " + stats.name + " (-" + std::to_string(stats.cost) + " Aur)");
                } catch (const EmpireException& e) {
                    logger.logError(e);
                }
            }
            return; // Blocăm restul input-urilor (mișcare/luptă) cât timp suntem în meniu
        }

        // 3. LOGICA DE MIȘCARE (Doar dacă meniul e închis)
        int dx = 0, dy = 0;
        if (IsKeyPressed(KEY_W)) dy = -1;
        else if (IsKeyPressed(KEY_S)) dy = 1;
        else if (IsKeyPressed(KEY_A)) dx = -1;
        else if (IsKeyPressed(KEY_D)) dx = 1;

        if (dx != 0 || dy != 0) {
            try {
                player.move(dx, dy, worldMap);
            } catch (const EmpireException& e) {
                logger.logError(e);
            }
        }

        // 4. ALTE ACȚIUNI (Luptă, Economie, Garnizoană)

        // TAB: Schimbă regiunea selectată pentru info/garnizoană
        if (IsKeyPressed(KEY_TAB)) {
            activeRegionIdx = (activeRegionIdx + 1) % regions.size();
        }

        // F: Atacă (Inamici pe hartă sau Asediu oraș)
        if (IsKeyPressed(KEY_F)) {
            handleCombat();
        }

        // N: Next Day (Colectează taxe, plătește upkeep, riscuri de rebeliune)
        if (IsKeyPressed(KEY_N)) {
            processNextDay();
        }

        // G: GARRISON - Lasă unitatea din spatele armatei în orașul curent
        if (IsKeyPressed(KEY_G)) {
            auto& sel = regions[activeRegionIdx];
            auto [px, py] = player.getPos();
            auto [cx, cy] = sel.getCity().getPos();

            if (px == cx && py == cy && sel.getCity().isOccupied()) {
                auto u = player.getArmy().popUnit();
                if (u) {
                    std::string n = u->getName();
                    sel.getCity().addUnitToGarrison(std::move(u));
                    logger.add("Transferat in garnizoana: " + n);
                } else {
                    logger.add("! Nu ai trupe de lasat.");
                }
            }
        }
        if (IsKeyPressed(KEY_U)) {
            auto& selCity = regions[activeRegionIdx].getCity();
            int cost = selCity.getUpgradeCost();

            if (selCity.isOccupied() && player.getGold() >= cost) {
                player.removeGold(cost);
                selCity.upgrade();
                logger.add("Ai modernizat " + selCity.getName() + " la nivelul " + std::to_string(selCity.getCityLevel()));
            } else {
                logger.add("Resurse insuficiente sau orasul nu iti apartine!");
            }
        }
        // T: TAKE - Retrage ultima unitate din garnizoana orașului în armata ta
        if (IsKeyPressed(KEY_T)) {
            auto& sel = regions[activeRegionIdx];
            auto [px, py] = player.getPos();
            auto [cx, cy] = sel.getCity().getPos();

            if (px == cx && py == cy && sel.getCity().isOccupied()) {
                try {
                    int limit = player.getUnitLimit(regions);
                    if ((int)player.getArmy().getUnits().size() < limit) {
                        auto u = sel.getCity().extractUnit();
                        if (u) {
                            std::string n = u->getName();
                            player.getArmy().addUnit(std::move(u));
                            logger.add("Retras din garnizoana: " + n);
                        } else {
                            logger.add("! Garnizoana este goala.");
                        }
                    } else {
                        throw PopulationLimitException("Nu ai loc in armata pentru retragere!");
                    }
                } catch (const EmpireException& e) {
                    logger.logError(e);
                }
            }
        }
    }
    // --- UI RENDERING ---
    void drawUI() {
        const int sbX = 1200; // Sidebar-ul începe la 1200px
        const int mapOffX = 50, mapOffY = 50;

        // 1. DESENARE HARTĂ (TEMA 1: Relief Diversificat)
        worldMap.draw(mapOffX, mapOffY);

        // 2. DESENARE CETĂȚI ȘI SELECȚIE
        for (size_t i = 0; i < regions.size(); ++i) {
            auto& city = regions[i].getCity();
            auto [cx, cy] = city.getPos();
            int sx = mapOffX + cx * 40 + 20;
            int sy = mapOffY + cy * 40 + 20;

            // Evidențiere selecție TAB
            if (i == (size_t)activeRegionIdx) {
                DrawCircleLines(sx, sy, 26, GOLD);
                DrawCircleLines(sx, sy, 27, YELLOW);
            }

            // Simbol Cetate: Verde (Ta), Maro (Inamică/Răsculată)
            Color cCol = city.isOccupied() ? GREEN : MAROON;
            DrawCircle(sx, sy, 15, cCol);
            DrawCircleLines(sx, sy, 16, RAYWHITE);

            // Numele cetății sub ea (MeasureText pentru centrare)
            const char* nameStr = city.getName().c_str();
            DrawText(nameStr, sx - MeasureText(nameStr, 12) / 2, sy + 22, 12, RAYWHITE);
            DrawText(TextFormat("Lvl %i", city.getCityLevel()), sx - 15, sy - 35, 10, SKYBLUE);
        }

        // 3. AVATAR JUCĂTOR
        auto [px, py] = player.getPos();
        DrawCircle(mapOffX + px * 40 + 20, mapOffY + py * 40 + 20, 12, GOLD);

        // 4. SIDEBAR - STATUS IMPERIU
        DrawRectangle(sbX, 0, 400, 1000, raylib::Color(25, 25, 30, 255));
        DrawLine(sbX, 0, sbX, 1000, GOLD);

        // --- Economie Netă ---
        DrawText("TEZAURUL COROANEI", sbX + 20, 30, 22, GOLD);
        DrawText(TextFormat("Aur actual: %i", player.getGold()), sbX + 20, 65, 20, WHITE);

        int income = calculateTotalIncome();
        int upkeep = calculateTotalUpkeep(); // Include mobile + garnizoane
        int netProfit = income - upkeep;

        DrawText(TextFormat("Profit Net: %+i", netProfit), sbX + 20, 90, 20, (netProfit >= 0 ? GREEN : RED));
        DrawLine(sbX + 20, 125, sbX + 380, 125, DARKGRAY);

        // --- Detalii Cetate & Risc Rebeliune ---
        auto& sel = regions[activeRegionIdx].getCity();
        DrawText(sel.getName().c_str(), sbX + 20, 140, 28, GOLD);

        if (sel.isOccupied()) {
            int pop = sel.getPopulation();
            int gCount = (int)sel.getGarrison().size();
            int needed = pop / 100; if (needed < 1) needed = 1;

            // Calcul risc rebeliune (Formula: (Needed - Current) / Needed)
            float risk = 0;
            if (gCount < needed) risk = (float)(needed - gCount) / needed;

            DrawText(TextFormat("Populatie: %i", pop), sbX + 20, 185, 18, LIGHTGRAY);
            DrawText(TextFormat("Garnizoana: %i / %i unitati", gCount, needed), sbX + 20, 210, 18, (risk > 0 ? ORANGE : GREEN));

            // Bara de Risc
            DrawRectangle(sbX + 20, 235, 300, 10, DARKGRAY);
            DrawRectangle(sbX + 20, 235, (int)(300 * risk), 10, RED);
            DrawText(TextFormat("RISC RASCOALA: %i%%", (int)(risk * 100)), sbX + 20, 250, 16, (risk > 0.5 ? RED : ORANGE));

            DrawText(TextFormat("[U] Upgrade (%i Aur)", sel.getUpgradeCost()), sbX + 20, 285, 18, SKYBLUE);
        } else {
            DrawText("STARE: NEOCUPATA", sbX + 20, 185, 20, MAROON);
        }

        // --- Armata Mobila (Trupele tale) ---
        DrawLine(sbX + 20, 320, sbX + 380, 320, DARKGRAY);
        DrawText("ARMATA MOBILA", sbX + 20, 335, 20, GOLD);
        int unitY = 370;
        auto counts = player.getArmy().getUnitCounts();
        for (auto const& [name, count] : counts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 35, unitY, 17, WHITE);
            unitY += 22;
        }

        // --- Jurnal de Razboi ---
        DrawText("WAR JOURNAL", sbX + 20, 720, 20, GOLD);
        int logY = 755;
        for (const auto& msg : logger.getMessages()) {
            DrawText(msg.c_str(), sbX + 20, logY, 15, LIGHTGRAY);
            logY += 20;
        }

        // 5. OVERLAY RECRUTARE (Tasta R)
        if (showRecruitment) {
            DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 200});
            DrawRectangle(300, 200, 1000, 600, {30, 30, 35, 255});
            DrawRectangleLines(300, 200, 1000, 600, GOLD);
            DrawText("RECRUTARE MERCENARI / GARNIZOANA", 550, 230, 30, GOLD);

            auto drawRow = [&](int idx, UnitType t, int y) {
                auto s = GameData[t];
                DrawText(TextFormat("[%i] %s", idx, s.name.c_str()), 350, y, 22, WHITE);
                DrawText(TextFormat("ATK: %i | HP: %i", s.atk, s.hp), 580, y, 20, RED);
                DrawText(TextFormat("UPKEEP: %i/zi", s.upkeep), 820, y, 20, ORANGE);
                DrawText(TextFormat("COST: %i", s.cost), 1100, y, 20, GREEN);
            };

            drawRow(1, UnitType::INFANTERIE, 320);
            drawRow(2, UnitType::ARCASI, 400);
            drawRow(3, UnitType::CAVALERIE, 480);
            drawRow(4, UnitType::GARDA, 560); // Aceasta poate servi ca "Garrison unit"
        }

        // 6. ECRAN GAME OVER (Pierdere totală de cetăți)
        if (gameOver) {
            DrawRectangle(0, 0, 1600, 1000, {150, 0, 0, 180});
            DrawText("IMPERIUL A CAZUT", 550, 450, 60, RAYWHITE);
            DrawText("Nu mai controlezi nicio cetate sau tezaurul a secat.", 540, 520, 20, LIGHTGRAY);
            DrawText("APASA ESC PENTRU A PARASI JOCUL", 620, 580, 18, GOLD);
        }

        // 7. FOOTER
        DrawRectangle(0, 930, 1200, 70, {0, 0, 0, 150});
        DrawText("[W,A,S,D] Miscare | [R] Recrutare | [TAB] Navigare | [U] Upgrade | [N] Zi Noua", 40, 945, 18, WHITE);
        DrawText("[G] Depoziteaza in Garnizoana | [T] Retrage Unitati in Armata Mobila", 40, 970, 16, GRAY);
        DrawText(TextFormat("ZIUA: %i / 50", clock.getDay()), 1050, 955, 24, GOLD);
    }
    // ==========================================================
// 15. LOGIN UI (Sistem de Autentificare Simplu)
// ==========================================================
class LoginUI {
private:
    char nameInput[32] = "\0";
    int letterCount = 0;
    bool authenticated = false;

public:
    void update() {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (letterCount < 31)) {
                nameInput[letterCount] = (char)key;
                nameInput[letterCount + 1] = '\0';
                letterCount++;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            letterCount--;
            if (letterCount < 0) letterCount = 0;
            nameInput[letterCount] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER) && letterCount > 0) authenticated = true;
    }

    void draw() const {
        DrawRectangle(0, 0, 1600, 1000, {20, 20, 25, 255});
        DrawText("EMPIRE RISING: RECRUITMENT & REBELLION", 450, 300, 35, GOLD);
        DrawText("INTRODUCETI NUMELE COMANDANTULUI:", 550, 420, 20, RAYWHITE);
        DrawRectangle(600, 470, 400, 50, LIGHTGRAY);
        DrawRectangleLines(600, 470, 400, 50, authenticated ? GREEN : GOLD);
        DrawText(nameInput, 610, 485, 25, BLACK);
        DrawText("APASATI [ENTER] PENTRU A INCEPE", 630, 550, 18, GRAY);
    }

    [[nodiscard]] bool isAuthenticated() const { return authenticated; }
    [[nodiscard]] std::string getPlayerName() const { return nameInput; }
};

// ==========================================================
// 16. CONTINUARE SIMULATION (BUCLA PRINCIPALA SI RENDER)
// ==========================================================
    // (Adaugă aceste metode în clasa Simulation)
public:
    Simulation() : window(1600, 1000, "EMPIRE RISING - TEMA 2"), currentState(GameState::LOGIN), worldMap(30, 20) {
        SetTargetFPS(60);
    }

    void run() {
        LoginUI login;
        while (!window.ShouldClose()) {
            if (currentState == GameState::LOGIN) {
                login.update();
                if (login.isAuthenticated()) {
                    player = Player(login.getPlayerName(), 1500);
                    initWorld();
                    currentState = GameState::SIMULATION;
                }
            } else if (currentState == GameState::SIMULATION) {
                handleInput();
            }

            BeginDrawing();
            window.ClearBackground({30, 30, 35, 255});

            if (currentState == GameState::LOGIN) {
                login.draw();
            } else if (currentState == GameState::SIMULATION) {
                drawUI();
            } else if (currentState == GameState::VICTORIE) {
                DrawRectangle(0, 0, 1600, 1000, {0, 100, 0, 200});
                DrawText("VICTORIE SUPREMA!", 600, 450, 50, GOLD);
                DrawText(TextFormat("Ai cucerit imperiul in %i zile.", clock.getDay()), 620, 520, 20, WHITE);
            } else if (currentState == GameState::DEFEAT) {
                DrawRectangle(0, 0, 1600, 1000, {100, 0, 0, 200});
                DrawText("INFRANGERE!", 650, 450, 50, RED);
                DrawText("Timpul a expirat sau ai pierdut toate cetatile.", 580, 520, 20, WHITE);
            }

            EndDrawing();
        }
    }
};

// ==========================================================
// 17. DEMO OBLIGATORIU TEMA 2 (CONSOLA) & MAIN
// ==========================================================
void RunRequirementsDemo() {
    std::cout << ">>> DEMO TEMA 2: VERIFICARE CERINTE <<<\n";

    // 1. Polimorfism & Clone
    std::unique_ptr<Unit> u1 = UnitFactory::CreateUnit(UnitType::EROU, "Achile");
    auto u2 = u1->clone();
    std::cout << "Original: " << *u1 << "\nClona: " << *u2 << "\n";

    // 2. dynamic_cast
    if (auto* h = dynamic_cast<Hero*>(u1.get())) {
        std::cout << "dynamic_cast reusit: " << h->getName() << " este Erou.\n";
    }

    // 3. Statice
    WorldClock::SetGlobalTaxRate(1.2f);
    std::cout << "Taxa globala setata la: " << WorldClock::GetGlobalTaxRate() << "\n";
    std::cout << "Total unitati create in memorie: " << Unit::getTotalUnits() << "\n";

    // 4. Exceptii
    try {
        throw PopulationLimitException("Test limita populatie.");
    } catch (const EmpireException& e) {
        std::cout << "Exceptie prinsa: " << e.what() << "\n";
    }
    std::cout << ">>> SFARSIT DEMO. Pornire interfata grafica...\n";
}

int main() {
    RunRequirementsDemo();
    try {
        Simulation game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "EROARE CRITICA: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}