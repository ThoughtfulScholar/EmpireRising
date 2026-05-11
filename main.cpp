#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>
#include <map>
#include <stdexcept>
#include <raylib-cpp.hpp>

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

    // --- METODE VIRTUALE PROTEJATE (Cerință Tema 2) ---
    [[nodiscard]] virtual int calculateTotalAttackImpl() const {
        return atk + (level * 5);
    }

    virtual void print(std::ostream& os) const {
        os << name << " [Lvl " << level << " | HP: " << hp << "/" << maxHp
           << " | ATK: " << calculateTotalAttack() << "]";
    }

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
    void display(std::ostream& os) const {
        print(os);
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
        const auto& s = GameData.at(type); // Sursa de date
        std::string fName = customName.empty() ? s.name : customName;

        switch (type) {
            // Trimitem HP, ATK și UPKEEP direct din GameData
            case UnitType::INFANTERIE:
                return std::make_unique<Hero>(fName, s.hp, s.atk, s.upkeep);
            case UnitType::ARCASI:
                return std::make_unique<Hero>(fName, s.hp, s.atk, s.upkeep);
            case UnitType::GARDA:
                return std::make_unique<GarrisonGuard>(fName); // Dacă Garda e specială
            case UnitType::EROU:
                return std::make_unique<Hero>(fName, s.hp, s.atk, s.upkeep);
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

    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "Oras: " << c.name << " (Pop: " << c.population << ")\n";
        os << "Garnizoana: ";
        for(const auto& u : c.garrison) {
            os << *u << " | "; // Compunere de apeluri: City apelează << de la Unit
        }
        return os;
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

    [[nodiscard]] std::string getName() const {
        // Extragem doar numărul din "Cetate Inamica 1" sau "Cetate Inamica 2"
        // Căutăm ultima cifră din string-ul 'name'
        std::string idOnly = "";
        for (char c : this->name) {
            if (isdigit(c)) {
                idOnly += c;
            }
        }

        // Dacă din vreun motiv nu găsește cifre, returnăm numele original să nu crape
        if (idOnly.empty()) return this->name;

        if (this->occupied) {
            return "Cetate Aliata " + idOnly;
        } else {
            return "Cetate Inamica " + idOnly;
        }
    }
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

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Comandant: " << p.commanderName << " | Tezaur: " << p.gold << " aur\n";
        return os;
    }

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
    bool showGarrisonMenu = false;
    bool showTakeMenu = false;

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
    template <typename T>
        int findUnitIndexByType(const std::vector<std::unique_ptr<Unit>>& units) {
            for (int i = 0; i < (int)units.size(); ++i) {
                if (dynamic_cast<T*>(units[i].get())) return i;
            }
            return -1;
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
        // 1. GESTIONARE MENIURI ACTIVE (R, G, T)
        if (showRecruitment || showGarrisonMenu || showTakeMenu) {

            // Toggle: Se închid doar la apăsarea aceleiași taste
            if (IsKeyPressed(KEY_R) && showRecruitment) { showRecruitment = false; return; }
            if (IsKeyPressed(KEY_G) && showGarrisonMenu) { showGarrisonMenu = false; return; }
            if (IsKeyPressed(KEY_T) && showTakeMenu) { showTakeMenu = false; return; }

            // --- SUB-MENIU G (PUNE ÎN GARNIZOANĂ) ---
            if (showGarrisonMenu) {
                UnitType toMove;
                bool keyHit = false;
                if (IsKeyPressed(KEY_ONE))   { toMove = UnitType::INFANTERIE; keyHit = true; }
                else if (IsKeyPressed(KEY_TWO))   { toMove = UnitType::ARCASI;     keyHit = true; }
                else if (IsKeyPressed(KEY_THREE)) { toMove = UnitType::CAVALERIE;  keyHit = true; }
                else if (IsKeyPressed(KEY_FOUR))  { toMove = UnitType::GARDA;      keyHit = true; }
                else if (IsKeyPressed(KEY_FIVE))  { toMove = UnitType::EROU;       keyHit = true; }

                if (keyHit) {
                    auto& pUnits = player.getArmy().getUnits();
                    std::string targetName = GameData[toMove].name; // Numele pe care îl căutăm

                    auto it = std::find_if(pUnits.begin(), pUnits.end(), [&](const std::unique_ptr<Unit>& u) {
                        // Verificare dublă: dynamic_cast (pentru siguranță) SAU comparare de nume
                        if (toMove == UnitType::INFANTERIE) return (dynamic_cast<Infantry*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toMove == UnitType::ARCASI)     return (dynamic_cast<Archer*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toMove == UnitType::CAVALERIE)  return (dynamic_cast<Cavalry*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toMove == UnitType::GARDA)      return (dynamic_cast<GarrisonGuard*>(u.get()) != nullptr || u->getName() == targetName);
                        if (toMove == UnitType::EROU)       return (dynamic_cast<Hero*>(u.get()) != nullptr || u->getName() == targetName);
                        return false;
                    });

                    if (it != pUnits.end()) {
                        regions[activeRegionIdx].getCity().addUnitToGarrison(std::move(*it));
                        pUnits.erase(it);
                        logger.add("Transferat in oras: " + targetName);
                    } else {
                        logger.add("! Nu ai " + targetName + " in armata.");
                    }
                }
            }

            // --- SUB-MENIU T (RETRAGE DIN GARNIZOANĂ) ---
            // --- SUB-MENIU T (RETRAGE DIN GARNIZOANĂ) ---
            if (showTakeMenu) {
                UnitType toTake;
                bool keyHit = false;
                if (IsKeyPressed(KEY_ONE))   { toTake = UnitType::INFANTERIE; keyHit = true; }
                else if (IsKeyPressed(KEY_TWO))   { toTake = UnitType::ARCASI;     keyHit = true; }
                else if (IsKeyPressed(KEY_THREE)) { toTake = UnitType::CAVALERIE;  keyHit = true; }
                else if (IsKeyPressed(KEY_FOUR))  { toTake = UnitType::GARDA;      keyHit = true; }
                else if (IsKeyPressed(KEY_FIVE))  { toTake = UnitType::EROU;       keyHit = true; }

                if (keyHit) {
                    try {
                        int limit = player.getUnitLimit(regions);
                        if ((int)player.getArmy().getUnits().size() >= limit)
                            throw PopulationLimitException("Armata mobila este plina!");

                        auto& gUnits = regions[activeRegionIdx].getCity().getGarrison();
                        std::string targetName = GameData[toTake].name;

                        auto it = std::find_if(gUnits.begin(), gUnits.end(), [&](const std::unique_ptr<Unit>& u) {
                            // Aceeași verificare hibridă
                            if (toTake == UnitType::INFANTERIE) return (dynamic_cast<Infantry*>(u.get()) != nullptr || u->getName() == targetName);
                            if (toTake == UnitType::ARCASI)     return (dynamic_cast<Archer*>(u.get()) != nullptr || u->getName() == targetName);
                            if (toTake == UnitType::CAVALERIE)  return (dynamic_cast<Cavalry*>(u.get()) != nullptr || u->getName() == targetName);
                            if (toTake == UnitType::GARDA)      return (dynamic_cast<GarrisonGuard*>(u.get()) != nullptr || u->getName() == targetName);
                            if (toTake == UnitType::EROU)       return (dynamic_cast<Hero*>(u.get()) != nullptr || u->getName() == targetName);
                            return false;
                        });

                        if (it != gUnits.end()) {
                            player.getArmy().addUnit(std::move(*it));
                            gUnits.erase(it);
                            logger.add("Recuperat din oras: " + targetName);
                        } else {
                            logger.add("! In oras nu exista " + targetName);
                        }
                    } catch (const EmpireException& e) { logger.logError(e); }
                }
            }

            // --- LOGICA RECRUTARE ---
            if (showRecruitment) {
                int typeIdx = -1;
                if (IsKeyPressed(KEY_ONE)) typeIdx = (int)UnitType::INFANTERIE;
                else if (IsKeyPressed(KEY_TWO)) typeIdx = (int)UnitType::ARCASI;
                else if (IsKeyPressed(KEY_THREE)) typeIdx = (int)UnitType::CAVALERIE;
                else if (IsKeyPressed(KEY_FOUR)) typeIdx = (int)UnitType::GARDA;
                else if (IsKeyPressed(KEY_FIVE)) typeIdx = (int)UnitType::EROU;
                if (typeIdx != -1) {
                    UnitType t = static_cast<UnitType>(typeIdx);
                    try {
                        player.recruit(UnitFactory::CreateUnit(t, GameData[t].name), GameData[t].cost, player.getUnitLimit(regions));
                        logger.add("Recrutat: " + GameData[t].name);
                    } catch (const EmpireException& e) { logger.logError(e); }
                }
            }
            return; // Blochează mișcarea când orice meniu e deschis
        }

        // 2. DETECTARE TINTĂ (PENTRU F)
        auto [px, py] = player.getPos();
        targetedEnemy = nullptr;
        for (auto& enemy : roamingEnemies) {
            if (!enemy.isDefeated() && std::abs(enemy.getX() - px) <= 1 && std::abs(enemy.getY() - py) <= 1) {
                targetedEnemy = &enemy;
                break;
            }
        }

        // 3. NAVIGARE ȘI COMNE GENERALE
        // 3. NAVIGARE (Cu Try-Catch pentru a preveni crash-ul la coliziune)
        int dx = 0, dy = 0;
        if (IsKeyPressed(KEY_W)) dy = -1;
        else if (IsKeyPressed(KEY_S)) dy = 1;
        else if (IsKeyPressed(KEY_A)) dx = -1;
        else if (IsKeyPressed(KEY_D)) dx = 1;

        if (dx != 0 || dy != 0) {
            try {
                // Verificăm dacă un inamic ocupă tile-ul următor (ca să nu trecem prin el)
                auto [px, py] = player.getPos();
                bool blockedByEnemy = std::any_of(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
                    return e.getX() == (px + dx) && e.getY() == (py + dy) && !e.isDefeated();
                });

                if (blockedByEnemy) {
                    logger.add("! DRUM BLOCAT: Inamicul iti bareaza calea.");
                } else {
                    // Aici se aruncă excepția dacă terenul e invalid
                    player.move(dx, dy, worldMap);
                }
            } catch (const EmpireException& e) {
                // Prindem eroarea (ex: "Nu poti trece prin munte!")
                // și o afișăm în log în loc să lăsăm jocul să crape
                logger.logError(e);
            }
        }

        if (IsKeyPressed(KEY_TAB)) activeRegionIdx = (activeRegionIdx + 1) % regions.size();
        if (IsKeyPressed(KEY_N)) processNextDay();
        if (IsKeyPressed(KEY_R)) { showRecruitment = true; return; }

        // 4. LOGICA DE LUPTĂ [F]
        if (IsKeyPressed(KEY_F)) {
            if (targetedEnemy && !targetedEnemy->isDefeated()) {
                try {
                    if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe!");
                    int pAtk = 0;
                    for(const auto& u : player.getArmy().getUnits()) pAtk += u->calculateTotalAttack();
                    targetedEnemy->takeDamage(pAtk);
                    if (!targetedEnemy->isDefeated()) {
                        player.getArmy().getFrontUnit()->takeDamage(targetedEnemy->getAtk());
                        player.getArmy().removeDeadUnits();
                    } else {
                        player.earnGold(250);
                        logger.add("VICTORIE! Armata distrusa.");
                        targetedEnemy = nullptr;
                    }
                } catch (const EmpireException& e) { logger.logError(e); }
            } else {
                auto& sel = regions[activeRegionIdx];
                if (px == sel.getCity().getPos().first && py == sel.getCity().getPos().second) {
                    try {
                        // VERIFICARE CRASH: Dacă orașul e inamic dar nu are trupe (bug-ul tău)
                        if (!sel.getCity().isOccupied()) {
                            if (sel.getEnemyGarrison().getUnits().empty()) {
                                // Generăm manual câteva unități inamice ca să avem cu cine să ne luptăm
                                try {
                                    sel.getEnemyGarrison().addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Infanterie Inamica"));
                                    sel.getEnemyGarrison().addUnit(UnitFactory::CreateUnit(UnitType::ARCASI, "Arcasi Inamici"));
                                    logger.add("Garnizoana inamica s-a regrupat pentru aparare!");
                                } catch (...) {
                                    logger.add("Eroare la generarea apararii inamice.");
                                }
                            }
                        }

                        if (player.getArmy().isEmpty()) throw CombatException("Nu ai trupe!");

                        // Acum e sigur să chemăm bătălia
                        logger.add(sel.executeBattleRound(player.getArmy()));
                    } catch (const EmpireException& e) {
                        logger.logError(e);
                    }
                }
            }
        }

        // 5. ACTIVARE ORAȘ (G/T/U) - REPARAT
        bool foundCityUnderPlayer = false;

        // Căutăm prin TOATE regiunile să vedem dacă jucătorul e pe vreuna
        for (int i = 0; i < (int)regions.size(); i++) {
            auto [cx, cy] = regions[i].getCity().getPos();
            if (px == cx && py == cy) {
                activeRegionIdx = i; // Actualizăm indexul la cetatea de sub picioarele noastre
                foundCityUnderPlayer = true;
                break;
            }
        }

        // Acum folosim regiunea proaspăt detectată
        auto& currentReg = regions[activeRegionIdx];

        if (foundCityUnderPlayer && currentReg.getCity().isOccupied()) {
            if (IsKeyPressed(KEY_G)) showGarrisonMenu = true;
            if (IsKeyPressed(KEY_T)) showTakeMenu = true;

            if (IsKeyPressed(KEY_U)) {
                City& c = currentReg.getCity();
                if (c.getCityLevel() < 5) { // Limită de nivel
                    if (player.getGold() >= c.getUpgradeCost()) {
                        player.removeGold(c.getUpgradeCost()); // Folosește spendGold sau removeGold
                        c.upgrade();
                        logger.add("Modernizat: " + c.getName()); // Acum va zice numele cetății corecte
                    } else {
                        logger.add("Eroare: Aur insuficient!");
                    }
                } else {
                    logger.add("Eroare: Nivel maxim atins!");
                }
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
        DrawRectangle(sbX, 0, 1600 - sbX, 1000, {25, 25, 30, 255}); // Adaugă asta!
        DrawLine(sbX, 0, sbX, 1000, GOLD);

    // --- Secțiunea: TIMP (Calendar) ---
    DrawText("STATUS CAMPANIE", sbX + 20, 15, 18, SKYBLUE);
    int curDay = clock.getDay();
    DrawText(TextFormat("Ziua: %i / %i", curDay, MAX_DAYS), sbX + 20, 40, 22, (curDay > 40 ? RED : WHITE));
    DrawRectangle(sbX + 20, 68, 360, 4, DARKGRAY);
    float timePerc = (float)curDay / (float)MAX_DAYS;
    DrawRectangle(sbX + 20, 68, (int)(360 * timePerc), 4, SKYBLUE);

    DrawLine(sbX + 20, 85, sbX + 380, 85, DARKGRAY);

    // --- Secțiunea A: Economie ---
    DrawText("FINANTE IMPERIU", sbX + 20, 100, 20, GOLD);
    DrawText(TextFormat("Tezaur: %i Aur", player.getGold()), sbX + 20, 125, 22, WHITE);

    int income = calculateTotalIncome();
    int upkeep = calculateTotalUpkeep();
    int net = income - upkeep;

    DrawText(TextFormat("Profit Net: %+i / zi", net), sbX + 20, 155, 18, (net >= 0 ? GREEN : RED));
    DrawText(TextFormat("Venit (Taxe): +%i", income), sbX + 30, 175, 15, GRAY);
    DrawText(TextFormat("Cheltuieli (Upkeep): -%i", upkeep), sbX + 30, 195, 15, MAROON);

    DrawLine(sbX + 20, 220, sbX + 380, 220, DARKGRAY);

    // --- Secțiunea B: Inspecție Cetate (TAB) ---
    auto& selRegion = regions[activeRegionIdx];
    City& selCity = selRegion.getCity();
    DrawText(selCity.getName().c_str(), sbX + 20, 235, 24, GOLD);

    if (selCity.isOccupied()) {
        int pop = selCity.getPopulation();
        int lvl = selCity.getCityLevel();
        int upCost = selCity.getUpgradeCost();

        // TOATE COORDONATELE DE MAI JOS SUNT AJUSTATE (+70...100 pixeli față de vechiul cod)
        DrawText(TextFormat("Pop.: %i | NIVEL: %i", pop, lvl), sbX + 20, 270, 17, LIGHTGRAY);

        if (lvl < 5) {
            Color costColor = (player.getGold() >= upCost) ? GREEN : RED;
            DrawText(TextFormat("[U] Upgrade: %i Aur", upCost), sbX + 20, 295, 16, costColor);
        } else {
            DrawText("NIVEL MAXIM ATINS", sbX + 20, 295, 16, GOLD);
        }

        int gCount = (int)selCity.getGarrison().size();
        int needed = pop / 100; if (needed < 1) needed = 1;
        float risk = (gCount < needed) ? (float)(needed - gCount) / (float)needed : 0.0f;

        Color riskColor = (risk > 0.5f) ? RED : (risk > 0.0f ? ORANGE : GREEN);
        DrawText(TextFormat("RISC REVOLTA: %i%%", (int)(risk * 100)), sbX + 20, 320, 16, riskColor);

        DrawRectangle(sbX + 20, 340, 300, 8, DARKGRAY);
        if (risk > 0) DrawRectangle(sbX + 20, 340, (int)(300 * risk), 8, riskColor);

        DrawText("GARNIZOANA TA:", sbX + 20, 355, 16, SKYBLUE);
        auto gCounts = selCity.getGarrisonCounts();
        int gy = 380;
        if (gCounts.empty()) DrawText("- Goala (Pericol!) -", sbX + 40, gy, 16, RED);
        for (auto const& [name, count] : gCounts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, WHITE);
            gy += 22;
        }
    } else {
        DrawText("STARE: OCUPAT DE INAMICI", sbX + 20, 270, 17, MAROON);
        DrawText("GARDA DETECTATA:", sbX + 20, 295, 16, RED);
        auto eCounts = selRegion.getEnemyGarrison().getUnitCounts();
        int gy = 320;
        for (auto const& [name, count] : eCounts) {
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, gy, 16, ORANGE);
            gy += 22;
        }
    }

    // ======================================== ==================
    // SIDEBAR DINAMIC (Secțiunile C, D, E)
    // ==========================================================

    // --- Secțiunea C: INSPECTOR TINTA (Etajul 1: 450 - 650) ---
    DrawLine(sbX + 20, 450, sbX + 380, 450, DARKGRAY);
    DrawText("INSPECTIE TINTA", sbX + 20, 460, 20, RED);

    int nextSectionY = 580; // Unde va începe următoarea secțiune dacă nu e inamic

    if (targetedEnemy) {
        DrawText(TextFormat("Inamic la [%i, %i]", targetedEnemy->getX(), targetedEnemy->getY()), sbX + 20, 485, 15, LIGHTGRAY);
        DrawText(TextFormat("HP: %i | ATK: %i", targetedEnemy->getHP(), targetedEnemy->getAtk()), sbX + 20, 505, 17, WHITE);

        auto tCounts = targetedEnemy->getTroops().getUnitCounts();
        int ty = 530;
        for (auto const& [name, count] : tCounts) {
            if (ty > 620) break; // Nu lăsăm lista inamicului să invadeze armata ta
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, ty, 15, RAYWHITE);
            ty += 18;
        }
        DrawText("[F] ATAC", sbX + 250, 460, 16, YELLOW); // Punem butonul de atac sus, lângă titlu
        nextSectionY = 650; // Împingem armata ta mai jos pentru că avem inamic
    } else {
        DrawText("Nicio amenintare.", sbX + 20, 485, 16, GRAY);
        nextSectionY = 520; // Urcăm armata ta mai sus pentru că e loc liber
    }

    // --- Secțiunea D: ARMATA TA MOBILA (Etajul 2: Dinamic - 800) ---
    DrawLine(sbX + 20, nextSectionY, sbX + 380, nextSectionY, DARKGRAY);
    DrawText("ARMATA TA MOBILA", sbX + 20, nextSectionY + 15, 20, GOLD);

    auto pCounts = player.getArmy().getUnitCounts();
    int py2 = nextSectionY + 45;

    if (pCounts.empty()) {
        DrawText("- Fara soldati -", sbX + 40, py2, 16, GRAY);
    } else {
        for (auto const& [name, count] : pCounts) {
            if (py2 > 790) { // Limită strictă înainte de War Journal
                DrawText("...", sbX + 40, py2, 16, GRAY);
                break;
            }
            DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 40, py2, 16, WHITE);
            py2 += 22;
        }
    }

    // --- Secțiunea E: WAR JOURNAL (Etajul 3: 815 - Fundul ecranului) ---
    DrawRectangle(sbX + 15, 815, 370, 175, {20, 20, 20, 200}); // Fundal mai opac pentru contrast
    DrawRectangleLines(sbX + 15, 815, 370, 175, DARKGRAY);
    DrawText("WAR JOURNAL", sbX + 20, 825, 18, GOLD);

    auto allMessages = logger.getMessages();
    int logY = 855;
    int mCount = 0;
    for (auto it = allMessages.rbegin(); it != allMessages.rend(); ++it) {
        if (mCount >= 6) break;
        std::string displayMsg = *it;
        if (displayMsg.length() > 40) displayMsg = displayMsg.substr(0, 37) + "...";

        Color msgColor = (mCount == 0) ? RAYWHITE : (mCount < 3 ? LIGHTGRAY : GRAY);
        DrawText(displayMsg.c_str(), sbX + 25, logY, 13, msgColor);
        logY += 19;
        mCount++;
    }

        // 6. OVERLAY RECRUTARE (R) - INCLUDE UPKEEP DETALIAT
        if (showRecruitment) {
            DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 230});
            DrawRectangle(300, 200, 1000, 600, {30, 30, 35, 255});
            DrawRectangleLines(300, 200, 1000, 600, GOLD);
            DrawText("TABARA DE RECRUTARE", 630, 230, 30, GOLD);

            int ry = 320;

            // Lambda funcția acum ia doar tipul și tasta, restul le trage singură din GameData
            auto drawRow = [&](const char* key, UnitType type) {
                // Luăm referința la datele reale
                const auto& s = GameData[type];

                DrawText(TextFormat("[%s] %s", key, s.name.c_str()), 350, ry, 22, WHITE);
                DrawText(TextFormat("Atk: %i | HP: %i", s.atk, s.hp), 580, ry, 20, LIGHTGRAY);
                DrawText(TextFormat("Upkeep: %i / zi", s.upkeep), 800, ry, 20, ORANGE);

                // Verificăm dacă jucătorul își permite, ca să colorăm prețul
                Color costColor = (player.getGold() >= s.cost) ? GREEN : RED;
                DrawText(TextFormat("Cost: %i", s.cost), 1100, ry, 20, costColor);

                ry += 70;
            };

            // Apelăm rândurile folosind DOAR tipul din GameData
            drawRow("1", UnitType::INFANTERIE);
            drawRow("2", UnitType::ARCASI);
            drawRow("3", UnitType::CAVALERIE);
            drawRow("4", UnitType::GARDA);
            drawRow("5", UnitType::EROU);

            DrawText("APASA [R] PENTRU A INCHIDE MENIUL", 620, 750, 18, GRAY);
        }

        // 7. FOOTER
        DrawRectangle(0, 930, 1200, 70, {15, 15, 20, 220});
        DrawText("[W,A,S,D] Navigare | [N] Zi Noua | [F] Lupta | [R] Recrutare | [TAB] Selectie", 30, 945, 18, RAYWHITE);
        DrawText("[G] Depozitare Garnizoana | [T] Retragere Unitati | [U] Modernizare Oras", 30, 970, 16, GOLD);
        // --- UI PENTRU LASAT IN GARNIZOANA (G) ---
        // --- UI PENTRU LASAT IN GARNIZOANA (G) ---
        if (showGarrisonMenu) {
            DrawRectangle(450, 350, 350, 250, {20, 20, 25, 245}); // Inaltime 250
            DrawRectangleLines(450, 350, 350, 250, GOLD);

            DrawText("LASA IN GARNIZOANA:", 470, 370, 20, GOLD);
            DrawText("1. Infanterie", 490, 410, 18, WHITE);
            DrawText("2. Arcasi", 490, 440, 18, WHITE);
            DrawText("3. Cavalerie", 490, 470, 18, WHITE);
            DrawText("4. Garda", 490, 500, 18, WHITE); // Y la 500
            DrawText("5. Erou", 490, 530, 18, WHITE);

            DrawText("Apasa [G] pentru a inchide", 470, 560, 15, GRAY);
        }

        // --- UI PENTRU LUAT DIN GARNIZOANA (T) ---
        if (showTakeMenu) {
            DrawRectangle(450, 350, 350, 250, {25, 25, 30, 245}); // Inaltime 250
            DrawRectangleLines(450, 350, 350, 250, SKYBLUE);

            DrawText("RETRAGE IN ARMATA:", 470, 370, 20, SKYBLUE);
            DrawText("1. Infanterie", 490, 410, 18, WHITE);
            DrawText("2. Arcasi", 490, 440, 18, WHITE);
            DrawText("3. Cavalerie", 490, 470, 18, WHITE);
            DrawText("4. Garda", 490, 500, 18, WHITE); // Y la 500
            DrawText("5. Erou", 490, 530, 18, WHITE);

            DrawText("Apasa [T] pentru a inchide", 470, 560, 15, GRAY);
        }
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
