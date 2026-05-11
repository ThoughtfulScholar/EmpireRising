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

        // static float GetFloat(float min, float max) {
        //     std::uniform_real_distribution<float> dist(min, max);
        //     return dist(getEngine());
        // }
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
        //static int getTotalLogs() { return totalLogEntries; }
    };
}

// Inițializare atribut static
int GameEngine::Logger::totalLogEntries = 0;
// ==========================================================
// 4. ABILITY (Compunere - Cerință Tema 1)
// ==========================================================
/*
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
*/

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
    static void playAttackSound() {}

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
    //[[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getAtk() const { return atk; }
    [[nodiscard]] int getUpkeep() const { return upkeepCost; }
    // [[nodiscard]] int getLevel() const { return level; }

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
    explicit GarrisonGuard(const std::string& n) : Unit(n, 250, 40, 15) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<GarrisonGuard>(*this); }
};

class Hero : public Unit {
public:
    // Am scos 'int d' din lista de parametri
    Hero(const std::string& n, int h, int a, int u) : Unit(n, h, a, u) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Hero>(*this); }
    //static void inspire() {}
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
    /*
    void inspireHeroes() {
        for (auto& u : units) {
            if (auto* hero = dynamic_cast<Hero*>(u.get())) {
                hero->inspire();
            }
        }
    }*/

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

    // void clearGarrison() {
    //     garrison.clear();
    // }

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
    int totalHP;
    int totalAtk;

public:
    EnemyArmy(int x, int y, int difficulty) : posX(x), posY(y) {
        // Generăm trupe în funcție de dificultate
        for(int i = 0; i < difficulty; ++i) {
            troops.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Jafuitor"));
        }
        updateStats();
    }

    void updateStats() {
        int newHP = 0;
        int newAtk = 0;
        for (const auto& u : troops.getUnits()) {
            newHP += u->getHP(); // Adunăm viața RĂMASĂ a fiecărei unități
            newAtk += u->calculateTotalAttack();
        }
        this->totalHP = newHP;
        this->totalAtk = newAtk;
    }

    // Setteri și Getteri
    void moveTowards(int targetX, int targetY, const WorldMap& wm) {
        // Calculăm direcția dorită (-1, 0, sau 1)
        int dx = (targetX > posX) ? 1 : (targetX < posX ? -1 : 0);
        int dy = (targetY > posY) ? 1 : (targetY < posY ? -1 : 0);

        // Încercăm să ne mișcăm pe axa unde distanța este mai mare (pentru un aspect mai natural)
        if (std::abs(targetX - posX) > std::abs(targetY - posY)) {
            // Încercăm X primul
            if (dx != 0 && wm.isValidMove(posX + dx, posY)) {
                posX += dx;
            }
            // Dacă X e blocat de teren, încercăm Y
            else if (dy != 0 && wm.isValidMove(posX, posY + dy)) {
                posY += dy;
            }
        } else {
            // Încercăm Y primul
            if (dy != 0 && wm.isValidMove(posX, posY + dy)) {
                posY += dy;
            }
            // Dacă Y e blocat de teren, încercăm X
            else if (dx != 0 && wm.isValidMove(posX + dx, posY)) {
                posX += dx;
            }
        }
    }

    int getX() const { return posX; }
    int getY() const { return posY; }
    int getHP() const { return totalHP; }
    int getAtk() const { return totalAtk; }
    bool isDefeated() const { return troops.isEmpty() || totalHP <= 0; }
    ArmyManager& getTroops() { return troops; }

    void takeDamage(int dmg) {
        if (troops.isEmpty()) return;

        // 1. Aplicăm damage unității din prima linie
        auto frontUnit = troops.getFrontUnit();
        frontUnit->takeDamage(dmg);

        // 2. Dacă unitatea a murit, o scoatem din armată
        troops.removeDeadUnits();

        // 3. Recalculăm statisticile totale (HP și Atac) imediat
        updateStats();
    }

    void draw(int offX, int offY) const {
        int px = offX + posX * 40 + 20;
        int py = offY + posY * 40 + 20;
        DrawPoly({(float)px, (float)py}, 4, 16, 0, RED);
        DrawText(TextFormat("HP:%i", totalHP), px - 15, py + 18, 10, RED);
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
    [[nodiscard]] static int getUnitLimit(const std::vector<Zone>& regions) {
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
/*
class Enemy {
private:
    int x, y;
    //int strength;
public:
    Enemy(int x, int y, int strength) : x(x), y(y), strength(strength) {}
    int getX() const { return x; }
    int getY() const { return y; }
    //int getStrength() const { return strength; }
};
*/
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
    EnemyArmy* targetedEnemy = nullptr; // Inamicul de lângă jucător

    int activeRegionIdx = 0;
    bool showRecruitment = false;
    bool gameOver = false;

    static constexpr int MAX_DAYS = 50;
    //static constexpr int SIDEBAR_WIDTH = 400;

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

    // [[nodiscard]] int getNetGold() const {
    //     return calculateTotalIncome() - calculateTotalUpkeep();
    // }

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
    // void checkGameStatus() {
    //     bool allCaptured = std::all_of(regions.begin(), regions.end(), [](const Zone& z) {
    //         return z.getCity().isOccupied();
    //     });
    //
    //     bool noCitiesLeft = std::none_of(regions.begin(), regions.end(), [](const Zone& z) {
    //         return z.getCity().isOccupied();
    //     });
    //
    //     if (allCaptured) currentState = GameState::VICTORIE;
    //     else if (noCitiesLeft || clock.getDay() >= MAX_DAYS) currentState = GameState::DEFEAT;
    // }

    void processNextDay() {
        if (gameOver) return;

        clock.nextDay();
        logger.add("--- ZIUA " + std::to_string(clock.getDay()) + " ---");

        int totalIncome = 0;
        int totalUpkeep = 0;
        int occupiedCitiesCount = 0; // Calculată aici

        // 1. Upkeep Jucător
        totalUpkeep += player.getArmy().calculateTotalUpkeep();

        // 2. Procesare Orașe (Economie & Rebeliune)
        for (auto& reg : regions) {
            City& city = reg.getCity();
            if (city.isOccupied()) {
                occupiedCitiesCount++; // Incrementată aici
                city.growPopulation();
                totalIncome += (int)(city.collectTaxes() * WorldClock::GetGlobalTaxRate());
                totalUpkeep += city.getGarrisonUpkeep();

                // Logica matematică de rebeliune (1 unitate / 100 oameni)
                city.checkRebellion(logger);
            }
        }

        // 3. Rezultat Financiar
        int netProfit = totalIncome - totalUpkeep;
        player.earnGold(netProfit);
        logger.add(TextFormat("Economie: Net %+i Aur", netProfit));

        // 4. MIȘCARE INAMICI (Cu restricții de teren și coliziune cu cetăți)
        for (auto& enemy : roamingEnemies) {
            Zone* targetZone = nullptr;
            float minDistance = 999.0f;

            // Găsim cea mai apropiată cetate ocupată de player
            for (auto& reg : regions) {
                if (reg.getCity().isOccupied()) {
                    auto [cx, cy] = reg.getCity().getPos();
                    float dist = (float)std::sqrt(std::pow(cx - enemy.getX(), 2) + std::pow(cy - enemy.getY(), 2));
                    if (dist < minDistance) {
                        minDistance = dist;
                        targetZone = &reg;
                    }
                }
            }

            if (targetZone) {
                auto [tx, ty] = targetZone->getCity().getPos();

                if (minDistance < 1.5f) {
                    City& targetCity = targetZone->getCity();
                    logger.add("! ASEDIU: Inamicii asalteaza " + targetCity.getName() + "!");

                    if (targetCity.getGarrison().empty()) {
                        targetCity.setOccupied(false);
                        logger.add("!! " + targetCity.getName() + " a cazut (Fara aparare)!");
                    } else {
                        targetCity.extractUnit();
                        enemy.takeDamage(100);
                        logger.add("Garnizoana din " + targetCity.getName() + " a rezistat, dar a pierdut oameni.");
                    }
                } else {
                    enemy.moveTowards(tx, ty, worldMap);
                }
            }
        }

        // 5. SPAWNATOR
        if (clock.getDay() % 6 == 0) {
            int attempts = 0;
            while (attempts < 10) {
                int sx = (GameEngine::RandomGen::GetInt(0, 1) == 0) ? 0 : 29;
                int sy = GameEngine::RandomGen::GetInt(0, 19);

                if (worldMap.isValidMove(sx, sy)) {
                    roamingEnemies.emplace_back(sx, sy, 2 + (clock.getDay() / 10));
                    logger.add("ALERTA: Intruziune inamica detectata!");
                    break;
                }
                attempts++;
            }
        }

        // 6. Curățare Unități
        std::erase_if(roamingEnemies, [](const EnemyArmy& e) { return e.isDefeated(); });

        // 7. VERIFICĂRI FINALE (Win/Loss Logic)
        // Folosim variabila occupiedCitiesCount calculată la început pentru a evita eroarea "unused variable"
        bool allCaptured = (occupiedCitiesCount == (int)regions.size());

        if (allCaptured) {
            currentState = GameState::VICTORIE;
            gameOver = true;
            logger.add("VICTORIE: Ai unit toate tinuturile sub un singur steag!");
        }
        else if (occupiedCitiesCount <= 0 && clock.getDay() > 1) { // Am înlocuit stillOccupied cu occupiedCitiesCount
            currentState = GameState::DEFEAT;
            gameOver = true;
            logger.add("INFRANGERE: Ultimul bastion a cazut.");
        }
        else if (player.getGold() < -1000) {
            currentState = GameState::DEFEAT;
            gameOver = true;
            logger.add("INFRANGERE: Tezaurul este gol, armata a dezertat.");
        }
        else if (clock.getDay() >= MAX_DAYS) {
            currentState = GameState::DEFEAT;
            gameOver = true;
            logger.add("INFRANGERE: Timpul a expirat. Campania a esuat.");
        }
    }
    // --- LOGICA DE RECRUTARE (Meniul R) ---
    // void handleRecruitment() {
    //     if (!showRecruitment) return; // Dacă meniul nu e vizibil, nu recrutăm
    //
    //     int typeToRecruit = -1;
    //     if (IsKeyPressed(KEY_ONE)) typeToRecruit = (int)UnitType::INFANTERIE;
    //     if (IsKeyPressed(KEY_TWO)) typeToRecruit = (int)UnitType::ARCASI;
    //     if (IsKeyPressed(KEY_THREE)) typeToRecruit = (int)UnitType::CAVALERIE;
    //     if (IsKeyPressed(KEY_FOUR)) typeToRecruit = (int)UnitType::GARDA;
    //
    //     if (typeToRecruit != -1) {
    //         UnitType t = static_cast<UnitType>(typeToRecruit);
    //         auto stats = GameData[t];
    //         try {
    //             int limit = player.getUnitLimit(regions);
    //             player.recruit(UnitFactory::CreateUnit(t, "Soldat"), stats.cost, limit);
    //             logger.add("Recrutat: Soldat (" + std::to_string(stats.cost) + " aur)");
    //         } catch (const EmpireException& e) {
    //             logger.logError(e);
    //         }
    //     }
    // }

    // void tryRecruit(UnitType type, const std::string& name, int cost) {
    //     try {
    //         int limit = player.getUnitLimit(regions);
    //         player.recruit(UnitFactory::CreateUnit(type, name), cost, limit);
    //         logger.add("Recrutare reusita: " + name);
    //     } catch (const EmpireException& e) { logger.logError(e); }
    // }

    // --- LOGICA DE LUPTĂ ---
/*
    void handleCombat() {
        auto [px, py] = player.getPos();

        // Căutăm dacă suntem pe același tile cu un inamic roaming
        auto it = std::find_if(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
            return e.getX() == px && e.getY() == py;
        });

        if (it != roamingEnemies.end()) {
            try {
                if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe pentru lupta!");

                // Calculăm puterea totală a jucătorului
                int playerAtk = 0;
                for(const auto& u : player.getArmy().getUnits()) playerAtk += u->calculateTotalAttack();

                // Schimb de damage
                int enemyAtk = it->getAtk();
                it->takeDamage(playerAtk);

                // Inamicul ripostează dacă mai are viață
                if (!it->isDefeated()) {
                    // Aplicăm damage-ul unității din față a jucătorului
                    player.getArmy().getFrontUnit()->takeDamage(enemyAtk);
                    player.getArmy().removeDeadUnits();
                    logger.add(TextFormat("Lupta crancena! Ai dat %i dmg, ai primit %i.", playerAtk, enemyAtk));
                } else {
                    player.earnGold(300); // Pradă de război
                    logger.add("Victorie! Inamicul a fost nimicit pe campul de lupta.");
                }
                it->updateStats(); // Recalculăm HP/ATK inamic după pierderi
            } catch (const EmpireException& e) { logger.logError(e); }
            return;
        }

        // Restul logicii de asediu oraș (rămâne neschimbată)
        auto& sel = regions[activeRegionIdx];
        auto [cx, cy] = sel.getCity().getPos();
        if (px == cx && py == cy) {
            try {
                std::string result = sel.executeBattleRound(player.getArmy());
                logger.add(result);
            } catch (const EmpireException& e) { logger.logError(e); }
        }
    }*/

    void handleInput() {
        // 1. DACĂ MENIUL DE RECRUTARE ESTE DESCHIS (Blochează restul acțiunilor)
        if (showRecruitment) {
            if (IsKeyPressed(KEY_R)) {
                showRecruitment = false;
                return;
            }

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
            return;
        }

        // 2. DETECTARE INAMIC ADIACENT (Pentru Inspector și Luptă)
        auto [px, py] = player.getPos();
        targetedEnemy = nullptr; // Resetăm ținta la fiecare cadru

        for (auto& enemy : roamingEnemies) {
            // Verificăm dacă inamicul este pe același tile sau imediat lângă (sus, jos, stânga, dreapta)
            if (std::abs(enemy.getX() - px) <= 1 && std::abs(enemy.getY() - py) <= 1) {
                targetedEnemy = &enemy;
                break;
            }
        }

        // 3. LOGICA DE MIȘCARE CU BLOCARE (Restricții Teren + Inamici)
        int dx = 0, dy = 0;
        if (IsKeyPressed(KEY_W)) dy = -1;
        else if (IsKeyPressed(KEY_S)) dy = 1;
        else if (IsKeyPressed(KEY_A)) dx = -1;
        else if (IsKeyPressed(KEY_D)) dx = 1;

        if (dx != 0 || dy != 0) {
            int nextX = px + dx;
            int nextY = py + dy;

            // Verificăm dacă un inamic ocupă tile-ul următor
            bool blockedByEnemy = std::any_of(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
                return e.getX() == nextX && e.getY() == nextY && !e.isDefeated();
            });

            if (blockedByEnemy) {
                logger.add("! DRUM BLOCAT: O armata inamica iti bareaza calea. Lupta [F]!");
            } else {
                try {
                    // Mișcarea verifică și terenul (apa/munți) prin worldMap
                    player.move(dx, dy, worldMap);
                } catch (const EmpireException& e) {
                    logger.logError(e);
                }
            }
        }

        // 4. LOGICA DE LUPTĂ [F]
        if (IsKeyPressed(KEY_F)) {
            if (targetedEnemy) {
                // Duel cu armata mobilă de pe hartă
                try {
                    if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe pentru lupta!");

                    int pAtk = 0;
                    for(const auto& u : player.getArmy().getUnits()) pAtk += u->calculateTotalAttack();

                    targetedEnemy->takeDamage(pAtk);

                    if (!targetedEnemy->isDefeated()) {
                        // Inamicul ripostează dacă supraviețuiește
                        player.getArmy().getFrontUnit()->takeDamage(targetedEnemy->getAtk());
                        player.getArmy().removeDeadUnits();
                        logger.add("Conflict! Ai provocat " + std::to_string(pAtk) + " daune armatei inamice.");
                    } else {
                        player.earnGold(250); // Pradă de război
                        logger.add("VICTORIE! Armata inamica a fost eliminata (+250 Aur).");
                    }
                    targetedEnemy->updateStats();
                } catch (const EmpireException& e) { logger.logError(e); }
            } else {
                // Dacă nu e inamic lângă, verificăm dacă suntem pe o cetate pentru ASEDIE
                auto& sel = regions[activeRegionIdx];
                if (px == sel.getCity().getPos().first && py == sel.getCity().getPos().second) {
                    try {
                        std::string res = sel.executeBattleRound(player.getArmy());
                        logger.add(res);
                    } catch (const EmpireException& e) { logger.logError(e); }
                }
            }
        }

        // 5. GESTIONARE ORAȘE (TAB, Upgrade, Garnizoană)
        if (IsKeyPressed(KEY_TAB)) activeRegionIdx = (activeRegionIdx + 1) % regions.size();
        if (IsKeyPressed(KEY_R)) showRecruitment = true;
        if (IsKeyPressed(KEY_N)) processNextDay();

        auto& currentReg = regions[activeRegionIdx];
        bool onCity = (px == currentReg.getCity().getPos().first && py == currentReg.getCity().getPos().second);

        // G: Pune în Garnizoană
        if (IsKeyPressed(KEY_G) && onCity && currentReg.getCity().isOccupied()) {
            auto u = player.getArmy().popUnit();
            if (u) {
                std::string n = u->getName();
                currentReg.getCity().addUnitToGarrison(std::move(u));
                logger.add("Transferat in garnizoana: " + n);
            }
        }

        // T: Retrage din Garnizoană
        if (IsKeyPressed(KEY_T) && onCity && currentReg.getCity().isOccupied()) {
            try {
                int limit = player.getUnitLimit(regions);
                if ((int)player.getArmy().getUnits().size() < limit) {
                    auto u = currentReg.getCity().extractUnit();
                    if (u) {
                        player.getArmy().addUnit(std::move(u));
                        logger.add("Unitate retrasa in armata mobila.");
                    }
                } else throw PopulationLimitException("Nu mai ai loc in armata!");
            } catch (const EmpireException& e) { logger.logError(e); }
        }

        // U: Upgrade Oraș
        if (IsKeyPressed(KEY_U)) {
            City& c = currentReg.getCity();
            if (c.isOccupied() && player.getGold() >= c.getUpgradeCost()) {
                player.removeGold(c.getUpgradeCost());
                c.upgrade();
                logger.add("Cetatea " + c.getName() + " a fost modernizata.");
            }
        }
    }
    // --- UI RENDERING ---
    void drawUI() {
        const int sbX = 1200;
        const int mapOffX = 50, mapOffY = 50;

        // 1. DESENARE HARTĂ ȘI ENTITĂȚI
        worldMap.draw(mapOffX, mapOffY);

        // 2. DESENARE CETĂȚI ȘI SELECȚIE
        for (size_t i = 0; i < regions.size(); ++i) {
            const auto& city = regions[i].getCity();
            auto [cx, cy] = city.getPos();
            int sx = mapOffX + cx * 40 + 20;
            int sy = mapOffY + cy * 40 + 20;

            if (i == (size_t)activeRegionIdx) {
                DrawCircleLines(sx, sy, 26, GOLD);
                DrawCircleLines(sx, sy, 27, YELLOW);
            }

            Color cCol = city.isOccupied() ? GREEN : MAROON;
            DrawCircle(sx, sy, 15, cCol);
            DrawCircleLines(sx, sy, 16, RAYWHITE);
            DrawText(city.getName().c_str(), sx - MeasureText(city.getName().c_str(), 12) / 2, sy + 22, 12, RAYWHITE);
        }

        // 3. DESENARE INAMICI MOBILI
        for (const auto& enemy : roamingEnemies) {
            int ex = mapOffX + enemy.getX() * 40 + 20;
            int ey = mapOffY + enemy.getY() * 40 + 20;
            DrawPoly({(float)ex, (float)ey}, 4, 14, 0, RED);
            DrawPolyLines({(float)ex, (float)ey}, 4, 15, 0, WHITE);
            DrawText(TextFormat("%i HP", enemy.getHP()), ex - 15, ey - 25, 10, RED);
        }

        // 4. AVATAR JUCĂTOR
        auto [px, py] = player.getPos();
        DrawCircle(mapOffX + px * 40 + 20, mapOffY + py * 40 + 20, 12, GOLD);

        // 5. SIDEBAR - CONTROL PANEL
        DrawRectangle(sbX, 0, 400, 1000, {25, 25, 30, 255});
        DrawLine(sbX, 0, sbX, 1000, GOLD);

        // --- Secțiunea A: Economie (RESTAURATĂ) ---
        DrawText("FINANTE IMPERIU", sbX + 20, 30, 20, GOLD);
        DrawText(TextFormat("Tezaur: %i Aur", player.getGold()), sbX + 20, 60, 22, WHITE);

        int income = calculateTotalIncome();
        int upkeep = calculateTotalUpkeep();
        int net = income - upkeep;

        DrawText(TextFormat("Profit Net: %+i / zi", net), sbX + 20, 90, 18, (net >= 0 ? GREEN : RED));
        DrawText(TextFormat("Venit (Taxe): +%i", income), sbX + 30, 115, 15, GRAY);
        DrawText(TextFormat("Cheltuieli (Upkeep): -%i", upkeep), sbX + 30, 135, 15, MAROON);

        DrawLine(sbX + 20, 160, sbX + 380, 160, DARKGRAY);

        // --- Secțiunea B: Inspecție Cetate (TAB) ---
        auto& selRegion = regions[activeRegionIdx];
        City& selCity = selRegion.getCity();
        DrawText(selCity.getName().c_str(), sbX + 20, 175, 24, GOLD);

        if (selCity.isOccupied()) {
            int pop = selCity.getPopulation();
            int lvl = selCity.getCityLevel();
            int upCost = selCity.getUpgradeCost(); // <--- AI NEVOIE DE ASTA

            DrawText(TextFormat("Pop.: %i | NIVEL: %i", pop, lvl), sbX + 20, 205, 17, LIGHTGRAY);

            // --- AICI APARE COSTUL DE UPGRADE ---
            if (lvl < 5) { // Presupunând că nivelul maxim e 5
                Color costColor = (player.getGold() >= upCost) ? GREEN : RED;
                DrawText(TextFormat("[U] Upgrade: %i Aur", upCost), sbX + 20, 225, 16, costColor);
            } else {
                DrawText("NIVEL MAXIM ATINS", sbX + 20, 225, 16, GOLD);
            }

            // Afișare RISC (mutat puțin mai jos ca să facă loc)
            int gCount = (int)selCity.getGarrison().size();
            int needed = pop / 100; if (needed < 1) needed = 1;
            float risk = (gCount < needed) ? (float)(needed - gCount) / (float)needed : 0.0f;

            Color riskColor = (risk > 0.5f) ? RED : (risk > 0.0f ? ORANGE : GREEN);
            DrawText(TextFormat("RISC REVOLTA: %i%%", (int)(risk * 100)), sbX + 20, 245, 16, riskColor);

            // Bara Vizuală de Risc
            DrawRectangle(sbX + 20, 265, 300, 8, DARKGRAY);
            if (risk > 0) DrawRectangle(sbX + 20, 265, (int)(300 * risk), 8, riskColor);

            // Compoziție Garnizoană
            DrawText("GARNIZOANA TA:", sbX + 20, 275, 16, SKYBLUE);
            auto gCounts = selCity.getGarrisonCounts();
            int gy = 300;
            if (gCounts.empty()) DrawText("- Goala (Pericol!) -", sbX + 40, gy, 16, RED);
            for (auto const& [name, count] : gCounts) {
                DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, WHITE);
                gy += 22;
            }
        } else {
            DrawText("STARE: OCUPAT DE INAMICI", sbX + 20, 205, 17, MAROON);
            DrawText("GARDA DETECTATA:", sbX + 20, 230, 16, RED);
            auto eCounts = selRegion.getEnemyGarrison().getUnitCounts();
            int gy = 255;
            for (auto const& [name, count] : eCounts) {
                DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, ORANGE);
                gy += 22;
            }
        }

        // --- Secțiunea C: Inspector Inamic Apropiat (TARGET) ---
        DrawLine(sbX + 20, 420, sbX + 380, 420, DARKGRAY);
        DrawText("INSPECTIE TINTA", sbX + 20, 435, 20, RED);

        if (targetedEnemy) {
            DrawText(TextFormat("Armata Mobila la [%i, %i]", targetedEnemy->getX(), targetedEnemy->getY()), sbX + 20, 465, 15, LIGHTGRAY);
            DrawText(TextFormat("HP Total: %i | Atk: %i", targetedEnemy->getHP(), targetedEnemy->getAtk()), sbX + 20, 485, 17, WHITE);

            DrawText("COMPONENTA:", sbX + 20, 510, 16, ORANGE);
            auto tCounts = targetedEnemy->getTroops().getUnitCounts();
            int ty = 535;
            for (auto const& [name, count] : tCounts) {
                DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, ty, 16, RAYWHITE);
                ty += 22;
            }
            DrawText("APASA [F] PENTRU ATAC", sbX + 20, ty + 10, 17, YELLOW);
        } else {
            DrawText("Nicio amenintare imediata.", sbX + 20, 465, 16, GRAY);
        }

        // --- Secțiunea D: Armata Ta Mobila ---
        DrawLine(sbX + 20, 680, sbX + 380, 680, DARKGRAY);
        DrawText("ARMATA TA MOBILA", sbX + 20, 695, 20, GOLD);
        auto pCounts = player.getArmy().getUnitCounts();
        int py2 = 725;
        if (pCounts.empty()) DrawText("- Fara soldati -", sbX + 40, py2, 16, GRAY);
        for (auto const& [name, count] : pCounts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, py2, 16, WHITE);
            py2 += 22;
        }

        // --- Secțiunea E: War Journal ---
        DrawText("WAR JOURNAL", sbX + 20, 840, 18, GOLD);
        int logY = 870;
        for (const auto& msg : logger.getMessages()) {
            DrawText(msg.c_str(), sbX + 20, logY, 13, LIGHTGRAY);
            logY += 17;
        }

        // 6. OVERLAY RECRUTARE (R) - INCLUDE UPKEEP DETALIAT
        if (showRecruitment) {
            DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 230});
            DrawRectangle(300, 200, 1000, 600, {30, 30, 35, 255});
            DrawRectangleLines(300, 200, 1000, 600, GOLD);
            DrawText("TABARA DE RECRUTARE", 630, 230, 30, GOLD);

            int ry = 320;
            auto drawRow = [&](const char* key, const char* name, int cost, int u_cost, int atk, int hp) {
                DrawText(TextFormat("[%s] %s", key, name), 350, ry, 22, WHITE);
                DrawText(TextFormat("Atk: %i | HP: %i", atk, hp), 580, ry, 20, LIGHTGRAY);
                DrawText(TextFormat("Upkeep: %i / zi", u_cost), 800, ry, 20, ORANGE);
                DrawText(TextFormat("Cost: %i", cost), 1100, ry, 20, GREEN);
                ry += 70;
            };

            drawRow("1", "Infanterie", 150, 10, 15, 100);
            drawRow("2", "Arcasi", 200, 15, 25, 60);
            drawRow("3", "Cavalerie", 400, 30, 40, 150);
            drawRow("4", "Garda", 500, 25, 30, 200);

            DrawText("APASA [R] PENTRU A INCHIDE MENIUL", 620, 750, 18, GRAY);
        }

        // 7. FOOTER
        DrawRectangle(0, 930, 1200, 70, {15, 15, 20, 220});
        DrawText("[W,A,S,D] Navigare | [N] Zi Noua | [F] Lupta | [R] Recrutare | [TAB] Selectie", 30, 945, 18, RAYWHITE);
        DrawText("[G] Depozitare Garnizoana | [T] Retragere Unitati | [U] Modernizare Oras", 30, 970, 16, GOLD);
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
                DrawText("Timpul a expirat sau ai pierdut toate cetatile ori ai ramas fara bani, iar armata a dezertat.", 380, 520, 20, WHITE);
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
    if (const auto* h = dynamic_cast<Hero*>(u1.get())) {
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
