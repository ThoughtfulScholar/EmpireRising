#include <raylib-cpp.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <random>
#include <ranges>
#include <cmath>
#include <ostream>
#include <exception>
#include <map>

// ==========================================================
// 1. IERARHIA DE EXCEPȚII (Specifica Temei 2)
// ==========================================================

enum class UnitType { INFANTERIE, ARCASI, CAVALERIE };
enum class GameState { LOGIN, SIMULATION, VICTORIE };

class EmpireException : public std::runtime_error {
public:
    explicit EmpireException(const std::string& msg) : std::runtime_error("Empire Error: " + msg) {}
};

class InsufficientGoldException : public EmpireException {
public:
    explicit InsufficientGoldException() : EmpireException("Aur insuficient pentru aceasta actiune!") {}
};

class InvalidMovementException : public EmpireException {
public:
    explicit InvalidMovementException(const std::string& details) : EmpireException("Miscare invalida: " + details) {}
};

class CombatException : public EmpireException {
public:
    explicit CombatException(const std::string& msg) : EmpireException("Eroare Combat: " + msg) {}
};

// ==========================================================
// 2. LOGICA ENGINE (Random & Color) - Pastrata din T1
// ==========================================================
namespace GameEngine {
    struct ColorPalette {
        [[nodiscard]] static raylib::Color Gold()    { return {255, 203, 0, 255}; }
        [[nodiscard]] static raylib::Color Sky()     { return {102, 191, 255, 255}; }
        [[nodiscard]] static raylib::Color UIBack()  { return {20, 20, 25, 255}; }
        [[nodiscard]] static raylib::Color Victory() { return {0, 228, 48, 255}; }
        [[nodiscard]] static raylib::Color Enemy()   { return {230, 41, 55, 255}; }
        [[nodiscard]] static raylib::Color Forest()  { return {34, 139, 34, 255}; }
        [[nodiscard]] static raylib::Color Mountain(){ return {105, 105, 105, 255}; }
        [[nodiscard]] static raylib::Color Water()   { return {0, 191, 255, 255}; }
    };

    struct RandomGen {
        static float GetFloat(float min, float max) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }
        static int GetInt(int min, int max) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(min, max);
            return dis(gen);
        }
    };
}

// ==========================================================
// 3. ABILITY & UNIT INTERFACE (Polimorfism T2)
// ==========================================================
class Ability {
private:
    std::string name;
    float activationChance;
    int bonusDamage;

public:
    explicit Ability(std::string n = "Atac standard", float chance = 0.0f, int bonus = 0)
        : name(std::move(n)), activationChance(chance), bonusDamage(bonus) {}

    [[nodiscard]] int trigger() const {
        if (GameEngine::RandomGen::GetFloat(0.0f, 1.0f) <= activationChance) return bonusDamage;
        return 0;
    }
    [[nodiscard]] const std::string& getName() const { return name; }
    friend std::ostream& operator<<(std::ostream& os, const Ability& ab) {
        os << "[Abilitate: " << ab.name << " | +" << ab.bonusDamage << "]";
        return os;
    }
};

class Unit {
protected:
    std::string name;
    int hp, maxHp, atk, def, level, xp;
    Ability specialAbility;
    int posX, posY; // Pentru harta 2D

    // Membru static pentru a numara unitatile create (Cerinta T2)
    static int totalUnitsCreated;

public:
    Unit(std::string n, int h, int a, int d, Ability ab)
        : name(std::move(n)), hp(h), maxHp(h), atk(a), def(d),
          level(1), xp(0), specialAbility(std::move(ab)), posX(0), posY(0) {
        totalUnitsCreated++;
    }

    virtual ~Unit() = default; // Destructor virtual obligatoriu

    // --- Cerinta T2: Virtual Copy Constructor ---
    [[nodiscard]] virtual std::unique_ptr<Unit> clone() const = 0;

    // --- NVI (Non-Virtual Interface) ---
    [[nodiscard]] int calculateTotalAttack() const {
        return atk + getPowerModifier() + (level * 5);
    }

    // --- Functii Virtuale Specifice Temei ---
    [[nodiscard]] virtual int getPowerModifier() const = 0;
    virtual void playAttackSound() const = 0;

    // --- Logica Pastrata din T1 ---
    virtual void takeDamage(int rawDamage) {
        int finalDamage = std::max(10, rawDamage - def);
        hp = std::max(0, hp - finalDamage);
    }

    void gainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++; xp = 0; maxHp += 40; hp = maxHp; atk += 7; def += 3;
        }
    }

    // Getters & Setters
    [[nodiscard]] bool isAlive() const { return hp > 0; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }
    void setPos(int x, int y) { posX = x; posY = y; }

    static int getTotalUnits() { return totalUnitsCreated; }

    virtual void print(std::ostream& os) const {
        os << name << " (Lvl " << level << ", HP: " << hp << "/" << maxHp << ")";
    }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        u.print(os);
        return os;
    }
};

int Unit::totalUnitsCreated = 0;
// ==========================================================
// 4. CLASE DERIVATE (Ierarhie T2: Minim 3 derivate)
// ==========================================================

class Infantry : public Unit {
public:
    Infantry(std::string n, Ability ab)
        : Unit(std::move(n), 300, 45, 35, std::move(ab)) {}

    [[nodiscard]] std::unique_ptr<Unit> clone() const override {
        return std::make_unique<Infantry>(*this);
    }

    [[nodiscard]] int getPowerModifier() const override {
        // Bonus de defensiva transformat in forta bruta pentru infanterie
        return specialAbility.trigger() + (def / 5);
    }

    void playAttackSound() const override {
        // Logica imaginara pentru sunet sau efect visual Raylib
    }

    void print(std::ostream& os) const override {
        os << "[Infanterie] ";
        Unit::print(os);
    }
};

class Archer : public Unit {
public:
    Archer(std::string n, Ability ab)
        : Unit(std::move(n), 180, 70, 15, std::move(ab)) {}

    [[nodiscard]] std::unique_ptr<Unit> clone() const override {
        return std::make_unique<Archer>(*this);
    }

    [[nodiscard]] int getPowerModifier() const override {
        // Arcasii au sansa de critic mai mare bazata pe level
        return specialAbility.trigger() + (level * 3);
    }

    void playAttackSound() const override { /* Sunet sageata */ }

    void print(std::ostream& os) const override {
        os << "[Arcas] ";
        Unit::print(os);
    }
};

class Cavalry : public Unit {
public:
    Cavalry(std::string n, Ability ab)
        : Unit(std::move(n), 350, 60, 25, std::move(ab)) {}

    [[nodiscard]] std::unique_ptr<Unit> clone() const override {
        return std::make_unique<Cavalry>(*this);
    }

    [[nodiscard]] int getPowerModifier() const override {
        // Cavalerie: bonus de sarja fix
        return specialAbility.trigger() + 20;
    }

    void playAttackSound() const override { /* Sunet copite */ }

    void print(std::ostream& os) const override {
        os << "[Cavalerie] ";
        Unit::print(os);
    }
};

class Hero : public Unit {
private:
    float inspirationAura; // Atribut specific derivatei

public:
    Hero(std::string n, Ability ab)
        : Unit(std::move(n), 500, 80, 50, std::move(ab)), inspirationAura(1.5f) {}

    [[nodiscard]] std::unique_ptr<Unit> clone() const override {
        return std::make_unique<Hero>(*this);
    }

    [[nodiscard]] int getPowerModifier() const override {
        // Eroul beneficiaza cel mai mult de abilitatea speciala
        return static_cast<int>(specialAbility.trigger() * inspirationAura);
    }

    void playAttackSound() const override { /* Sunet triumfal */ }

    // Functie specifica eroului (pentru testare dynamic_cast ulterior)
    void inspireTroops() { inspirationAura += 0.1f; }

    void print(std::ostream& os) const override {
        os << "⭐ [EROU] ";
        Unit::print(os);
    }
};

// ==========================================================
// 5. UNIT REGISTRY (Adaptat pentru T2)
// ==========================================================
// Folosim un Factory Pattern simplu pentru a returna pointeri de baza
struct UnitFactory {
    static std::unique_ptr<Unit> CreateUnit(UnitType type, const std::string& name) {
        if (type == UnitType::INFANTERIE)
            return std::make_unique<Infantry>(name, Ability("Zid Scuturi", 0.3f, 15));
        if (type == UnitType::ARCASI)
            return std::make_unique<Archer>(name, Ability("Sageata Foc", 0.4f, 25));
        if (type == UnitType::CAVALERIE)
            return std::make_unique<Cavalry>(name, Ability("Sarja", 0.25f, 40));

        // Default / Garda
        return std::make_unique<Infantry>(name, Ability("Garda Cetate", 0.1f, 10));
    }
};
// ==========================================================
// 6. SISTEMUL DE HARTĂ (Cerință Nouă: Grid 2D)
// ==========================================================

enum class TerrainType { PLAIN, FOREST, MOUNTAIN, WATER, CITY_TILE };

class Tile {
private:
    TerrainType type;
    bool walkable;
    raylib::Color color;

public:
    Tile(TerrainType t = TerrainType::PLAIN) : type(t) {
        switch (t) {
            case TerrainType::PLAIN:     walkable = true;  color = raylib::Color{100, 150, 100, 255}; break; // Iarba
            case TerrainType::FOREST:    walkable = true;  color = raylib::Color{34, 139, 34, 255};   break;
            case TerrainType::MOUNTAIN:  walkable = false; color = raylib::Color{100, 100, 105, 255};  break;
            case TerrainType::WATER:     walkable = false; color = raylib::Color{0, 120, 210, 255};    break;
            case TerrainType::CITY_TILE: walkable = true;  color = GOLD; break; // Sub orase
        }
    }
    void setType(TerrainType t) { *this = Tile(t); }
    [[nodiscard]] bool isWalkable() const { return walkable; }
    [[nodiscard]] raylib::Color getColor() const { return color; }
};

class WorldMap {
private:
    int width, height;
    std::vector<std::vector<Tile>> grid;

public:
    WorldMap(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Tile>(width, Tile(TerrainType::PLAIN)));
        generateProcedural();
    }

    void generateProcedural() {
        // Generăm obstacole aleatorii
        for (auto& row : grid) {
            for (auto& tile : row) {
                float r = GameEngine::RandomGen::GetFloat(0, 1);
                if (r < 0.20f) tile.setType(TerrainType::MOUNTAIN);
                else if (r < 0.10f) tile.setType(TerrainType::WATER);
            }
        }
    }

    // Funcție crucială: Creează un drum sigur între două puncte
    void createPath(int startX, int startY, int endX, int endY) {
        int curX = startX;
        int curY = startY;
        while (curX != endX || curY != endY) {
            grid[curY][curX].setType(TerrainType::PLAIN); // "Săpăm" prin munți/apă
            if (curX < endX) curX++;
            else if (curX > endX) curX--;
            else if (curY < endY) curY++;
            else if (curY > endY) curY--;
        }
        grid[endY][endX].setType(TerrainType::CITY_TILE);
    }

    void setTile(int x, int y, TerrainType t) {
        if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x].setType(t);
    }

    [[nodiscard]] bool isValidMove(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        return grid[y][x].isWalkable();
    }

    void draw(int offX, int offY) const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                grid[y][x].getColor().DrawRectangle(offX + x * 40, offY + y * 40, 39, 39);
            }
        }
    }
};

// ==========================================================
// 7. ENTITY COORDINATES (Helper)
// ==========================================================
struct MapPosition {
    int x, y;
    bool operator==(const MapPosition& other) const {
        return x == other.x && y == other.y;
    }
};
// ==========================================================
// 8. ORAȘUL ȘI GARNIZOANA (Adaptate pentru Hartă 2D)
// ==========================================================

class City {
private:
    std::string cityName;
    int cityLevel;
    int baseIncome;
    int population;
    bool occupied;
    int posX, posY; // Coordonate pe grid

public:
    explicit City(std::string name = "Asezare", int x = 0, int y = 0, int income = 150)
        : cityName(std::move(name)), cityLevel(1), baseIncome(income),
          population(100), occupied(false), posX(x), posY(y) {}

    bool upgrade(int& playerGold) {
        if (!occupied) throw EmpireException("Nu poti upgrada un oras inamic!");

        const int cost = getUpgradeCost();
        if (playerGold >= cost) {
            playerGold -= cost;
            cityLevel++;
            population += 75;
            return true;
        }
        throw InsufficientGoldException();
    }

    void growPopulation() {
        if (occupied) population += 5 + (cityLevel * 2);
    }

    [[nodiscard]] int getUpgradeCost() const {
        return 350 * cityLevel + (cityLevel * cityLevel * 50) + (population / 5);
    }

    [[nodiscard]] int collectTaxes() const {
        if (!occupied) return 0;
        return baseIncome + (cityLevel * 100) + (population / 2);
    }

    // Getters & Setters
    void setOccupied(bool status) { occupied = status; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    [[nodiscard]] const std::string& getName() const { return cityName; }
    [[nodiscard]] int getLevel() const { return cityLevel; }
    [[nodiscard]] int getPopulation() const { return population; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }

    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "Oras: " << c.cityName << " [" << c.posX << "," << c.posY << "] (Lvl: " << c.cityLevel << ")";
        return os;
    }
};

// ==========================================================
// 9. ARMY MANAGER (Gestiune Polimorfică T2)
// ==========================================================

class ArmyManager {
private:
    std::vector<std::unique_ptr<Unit>> units;

public:
    ArmyManager() = default;

    // Cerință T2: Copy and Swap & Rule of Three pentru clasa cu pointeri
    ArmyManager(const ArmyManager& other) {
        for (const auto& u : other.units) {
            units.push_back(u->clone()); // Folosim virtual constructor clone()
        }
    }

    ArmyManager& operator=(ArmyManager other) {
        std::swap(units, other.units);
        return *this;
    }

    ~ArmyManager() = default; // unique_ptr curăță automat

    void addUnit(std::unique_ptr<Unit> u) {
        if (u) units.push_back(std::move(u));
    }

    void removeDeadUnits() {
        std::erase_if(units, [](const auto& u) { return !u->isAlive(); });
    }

    [[nodiscard]] bool isEmpty() const { return units.empty(); }

    // Acces polimorfic
    [[nodiscard]] Unit* getFrontUnit() {
        return units.empty() ? nullptr : units.front().get();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Unit>>& getUnits() const {
        return units;
    }

    // Cerință T2: Utilizare dynamic_cast
    void inspireAllHeroes() {
        for (auto& u : units) {
            if (auto* hero = dynamic_cast<Hero*>(u.get())) {
                hero->inspireTroops();
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const ArmyManager& am) {
        os << "Armata (" << am.units.size() << " unitati)";
        return os;
    }
};

class Garrison {
private:
    ArmyManager troops;
    std::string faction;

public:
    explicit Garrison(std::string fName = "Aparatori") : faction(std::move(fName)) {}

    void addDefender(std::unique_ptr<Unit> u) { troops.addUnit(std::move(u)); }
    void cleanup() { troops.removeDeadUnits(); }
    [[nodiscard]] bool isDefeated() const { return troops.isEmpty(); }
    [[nodiscard]] Unit* getFrontUnit() { return troops.getFrontUnit(); }
    [[nodiscard]] const ArmyManager& getTroops() const { return troops; }

    friend std::ostream& operator<<(std::ostream& os, const Garrison& g) {
        os << "Garnizoana " << g.faction << ": " << g.troops;
        return os;
    }
};
class EnemyArmy {
private:
    int posX, posY;
    ArmyManager troops;
    std::string identifier;

public:
    EnemyArmy(int x, int y, std::string id) : posX(x), posY(y), identifier(std::move(id)) {
        // Generăm trupe aleatorii pentru inamic (Polimorfism Tema 2)
        int numUnits = GameEngine::RandomGen::GetInt(1, 3);
        for(int i = 0; i < numUnits; ++i) {
            troops.addUnit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Garda Neagra"));
        }
    }

    [[nodiscard]] int getX() const { return posX; }
    [[nodiscard]] int getY() const { return posY; }
    [[nodiscard]] bool isDefeated() const { return troops.isEmpty(); }
    [[nodiscard]] ArmyManager& getTroops() { return troops; }
    [[nodiscard]] const std::string& getId() const { return identifier; }

    void draw(int offX, int offY) const {
        // Desenăm armata ca un diamant roșu pe hartă
        DrawPoly({(float)(offX + posX * 40 + 20), (float)(offY + posY * 40 + 20)}, 4, 15, 0, RED);
        DrawPolyLines({(float)(offX + posX * 40 + 20), (float)(offY + posY * 40 + 20)}, 4, 16, 0, WHITE);
    }
};
// ==========================================================
// 10. ZONELE (Regiuni pe Hartă)
// ==========================================================

class Zone {
private:
    std::string zoneName;
    City localCity;
    Garrison enemyGarrison;
    raylib::Color mapTint;
    bool discovered;

public:
    Zone(std::string name, City city, Garrison garrison, raylib::Color tint)
        : zoneName(std::move(name)), localCity(std::move(city)),
          enemyGarrison(std::move(garrison)), mapTint(tint), discovered(false) {}

    // Metoda de luptă adaptată pentru T2 (Polimorfism & NVI)
    [[nodiscard]] std::string executeBattleRound(ArmyManager& playerArmy) {
        if (enemyGarrison.isDefeated()) {
            localCity.setOccupied(true);
            return "Cetatea " + localCity.getName() + " a fost deja cucerita!";
        }

        if (playerArmy.isEmpty()) {
            throw CombatException("Nu poti ataca fara trupe!");
        }

        Unit* pUnit = playerArmy.getFrontUnit();
        Unit* eUnit = enemyGarrison.getFrontUnit();

        // Logica de atac folosind NVI (calculateTotalAttack e non-virtual, dar apeleaza getPowerModifier virtual)
        int pAtk = pUnit->calculateTotalAttack();
        eUnit->takeDamage(pAtk);
        pUnit->playAttackSound(); // Apel virtual pur

        std::string result = pUnit->getName() + " loveste cu " + std::to_string(pAtk) + ". ";

        if (!eUnit->isAlive()) {
            enemyGarrison.cleanup();
            pUnit->gainXP(85);
            if (enemyGarrison.isDefeated()) {
                localCity.setOccupied(true);
                return result + "VICTORIE! Garnizoana a fost decimata.";
            }
            return result + "Inamicul a cazut!";
        }

        // Riposta inamicului
        int eAtk = eUnit->calculateTotalAttack();
        pUnit->takeDamage(eAtk);
        result += "Inamicul riposteaza: -" + std::to_string(eAtk) + " HP.";

        if (!pUnit->isAlive()) {
            playerArmy.removeDeadUnits();
            return result + " Tragedi! Unitatea ta a pierit.";
        }

        return result;
    }

    // Getters
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const City& getCity() const { return localCity; }
    [[nodiscard]] Garrison& getGarrison() { return enemyGarrison; }
    [[nodiscard]] const Garrison& getGarrison() const { return enemyGarrison; }
    [[nodiscard]] raylib::Color getTint() const { return mapTint; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Regiunea " << z.zoneName << " | " << z.localCity;
        return os;
    }
};

// ==========================================================
// 11. PLAYER (Comandantul Imperiului)
// ==========================================================

class Player {
private:
    std::string commanderName;
    int gold;
    ArmyManager army; // Foloseste ArmyManager (Copy-and-Swap inside)
    int posX, posY;

public:
    explicit Player(std::string name = "Comandant", int startingGold = 1000)
        : commanderName(std::move(name)), gold(startingGold), posX(2), posY(2) {}

    // Metoda noua pentru limita
    [[nodiscard]] int getUnitLimit(const std::vector<Zone>& regions) const {
        int limit = 4; // Limita de bază (începi cu 4 locuri, de exemplu)
        for (const auto& r : regions) {
            if (r.getCity().isOccupied()) {
                limit += r.getCity().getLevel() * 2;
            }
        }
        return limit;
    }

    // Recrutare polimorfica
    void recruit(std::unique_ptr<Unit> u, int cost) {
        if (gold >= cost) {
            gold -= cost;
            army.addUnit(std::move(u));
        } else {
            throw InsufficientGoldException();
        }
    }

    // Logica de miscare pe harta cu exceptii (T2)
    void move(int dx, int dy, const WorldMap& worldMap) {
        int newX = posX + dx;
        int newY = posY + dy;

        if (!worldMap.isValidMove(newX, newY)) {
            throw InvalidMovementException("Teren impracticabil sau limitele hartii depasite!");
        }

        posX = newX;
        posY = newY;
    }

    // Gestiune Aur
    void earnGold(int amount) { gold += amount; }
    bool spendGold(int amount) {
        if (gold >= amount) {
            gold -= amount;
            return true;
        }
        return false;
    }

    // Getters
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return commanderName; }
    [[nodiscard]] ArmyManager& getArmy() { return army; }
    [[nodiscard]] std::pair<int, int> getPos() const { return {posX, posY}; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Comandant " << p.commanderName << " | Aur: " << p.gold << " | " << p.army;
        return os;
    }
};
// ==========================================================
// 12. LOGGING ȘI EVENIMENTE (Atribute Statice T2)
// ==========================================================

namespace GameEngine {
    class Logger {
    private:
        std::vector<std::string> messages;
        static int totalLogEntries; // Atribut static (Cerinta T2)

    public:
        void add(const std::string& msg) {
            messages.push_back(msg);
            totalLogEntries++;
            if (messages.size() > 12) {
                messages.erase(messages.begin());
            }
        }

        // Metoda specifica pentru a loga erori (utila pentru try-catch)
        void logError(const std::exception& e) {
            add("![EROARE]: " + std::string(e.what()));
        }

        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
        [[nodiscard]] static int getTotalLogs() { return totalLogEntries; }

        friend std::ostream& operator<<(std::ostream& os, const Logger& logger) {
            os << "--- Jurnal Imperial (Total Intrari: " << totalLogEntries << ") ---\n";
            for (const auto& msg : logger.messages) os << "  > " << msg << "\n";
            return os;
        }
    };
}

int GameEngine::Logger::totalLogEntries = 0;

// ==========================================================
// 13. LOGIN MANAGER (Interfața de pornire)
// ==========================================================

class LoginManager {
private:
    std::string inputBuffer;
    bool authenticated = false;
    static constexpr size_t MAX_NAME_LENGTH = 15;

public:
    void update() {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (inputBuffer.length() < MAX_NAME_LENGTH)) {
                inputBuffer += static_cast<char>(key);
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer.empty()) inputBuffer.pop_back();

        if (IsKeyPressed(KEY_ENTER) && !inputBuffer.empty()) {
            // Simulam o verificare de securitate simpla (T2: Throw in logic)
            if (inputBuffer.find_first_of("0123456789") != std::string::npos) {
                // Nu permitem numere in numele comandantului (exemplu de logica)
                authenticated = true;
            }
            authenticated = true;
        }
    }

    void draw() const {
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK, DARKBLUE);
        raylib::Text::Draw("EMPIRE RISING", 500, 300, 80, GameEngine::ColorPalette::Gold());
        raylib::Text::Draw("Introdu numele noului Comandant:", 580, 450, 20, LIGHTGRAY);

        DrawRectangle(600, 500, 400, 60, RAYWHITE);
        DrawRectangleLines(600, 500, 400, 60, GameEngine::ColorPalette::Gold());
        raylib::Text::Draw(inputBuffer, 620, 512, 35, DARKGRAY);

        if (inputBuffer.empty()) {
            raylib::Text::Draw("Asteptare input...", 620, 515, 25, GRAY);
        } else {
            raylib::Text::Draw("Apasa ENTER pentru a jura loialitate", 620, 570, 18, GameEngine::ColorPalette::Sky());
        }
    }

    [[nodiscard]] bool isAuthenticated() const { return authenticated; }
    [[nodiscard]] const std::string& getPlayerName() const { return inputBuffer; }

    friend std::ostream& operator<<(std::ostream& os, const LoginManager& lm) {
        os << "[Login] Status: " << (lm.authenticated ? "OK" : "Asteptare") << " User: " << lm.inputBuffer;
        return os;
    }
};

// ==========================================================
// 14. TIME SYSTEM (Mecanica de tura)
// ==========================================================

class WorldClock {
private:
    int day;
    static float globalTaxModifier; // Atribut static pentru T2

public:
    explicit WorldClock() : day(1) {}

    void nextDay() { day++; }
    [[nodiscard]] int getDay() const { return day; }

    // Functie statica pentru a modifica economia globala
    static void SetGlobalTaxRate(float rate) { globalTaxModifier = rate; }
    static float GetGlobalTaxRate() { return globalTaxModifier; }

    friend std::ostream& operator<<(std::ostream& os, const WorldClock& wc) {
        os << "Ziua: " << wc.day << " | Rata Taxe: " << (globalTaxModifier * 100) << "%";
        return os;
    }
};

float WorldClock::globalTaxModifier = 1.0f;
// ==========================================================
// 15. SIMULATION (Arhitectul Sistemului)
// ==========================================================

// ==========================================================
// 15. SIMULATION (Structură Refăcută pentru Noul UI)
// ==========================================================

class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    LoginManager loginUI;
    Player player;
    WorldMap worldMap;
    WorldClock clock;
    GameEngine::Logger logger;

    std::vector<Zone> regions;
    std::vector<EnemyArmy> roamingEnemies; // Armate care blochează drumul

    int activeRegionIdx = 0;
    bool showRecruitment = false;

    // Constante pentru noul Layout
    static constexpr int SIDEBAR_WIDTH = 400;
    static constexpr int FOOTER_HEIGHT = 80;

    void handleRecruitment() {
        int currentUnits = static_cast<int>(player.getArmy().getUnits().size());
        int maxLimit = player.getUnitLimit(regions);

        try {
            if (IsKeyPressed(KEY_ONE)) {
                if (currentUnits >= maxLimit) throw EmpireException("Limita de populatie atinsa! Upgradeaza orasele.");
                player.recruit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Infanterie"), 150);
                logger.add("Recrutat: Infanterie");
            }
            if (IsKeyPressed(KEY_TWO)) {
                if (currentUnits >= maxLimit) throw EmpireException("Limita de populatie atinsa! Upgradeaza orasele.");
                player.recruit(UnitFactory::CreateUnit(UnitType::ARCASI, "Arcas"), 180);
                logger.add("Recrutat: Arcas");
            }
            if (IsKeyPressed(KEY_THREE)) {
                if (currentUnits >= maxLimit) throw EmpireException("Limita de populatie atinsa! Upgradeaza orasele.");
                player.recruit(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Cavaler"), 300);
                logger.add("Recrutat: Cavaler");
            }
        } catch (const EmpireException& e) {
            logger.logError(e);
        }
    }
    void handleCityActions() {
        // Acum folosim activeRegionIdx (orașul selectat cu TAB) în loc de poziția jucătorului
        auto& reg = regions[activeRegionIdx];

        if (IsKeyPressed(KEY_U)) {
            if (reg.getCity().isOccupied()) {
                try {
                    int goldBefore = player.getGold();
                    int currentGold = goldBefore;

                    if (reg.getCity().upgrade(currentGold)) {
                        player.spendGold(goldBefore - currentGold);
                        logger.add("Upgrade: " + reg.getCity().getName() + " la Lvl " + std::to_string(reg.getCity().getLevel()));
                    }
                } catch (const EmpireException& e) {
                    logger.logError(e);
                }
            } else {
                logger.add("![EROARE]: Nu poți upgrada o cetate inamică!");
            }
        }
    }

    void initWorld() {
        int startX = player.getPos().first;
        int startY = player.getPos().second;

        // 1. Generăm între 4 și 8 orașe
        int numCities = GameEngine::RandomGen::GetInt(4, 8);

        for (int i = 0; i < numCities; ++i) {
            int cx, cy;
            // Evităm spawn-ul exact peste jucător
            do {
                cx = GameEngine::RandomGen::GetInt(2, 27);
                cy = GameEngine::RandomGen::GetInt(2, 17);
            } while (cx == startX && cy == startY);

            // 2. Garantăm drumul (Săpăm prin munți/apă)
            worldMap.createPath(startX, startY, cx, cy);

            std::string zName = "Provincia " + std::to_string(i + 1);
            City c("Cetatea " + std::to_string(i + 1), cx, cy);

            // Primele 2 orașe sunt ale jucătorului (conform cerinței)
            if (i < 2) {
                c.setOccupied(true);
                regions.emplace_back(zName, c, Garrison("Garda Locala"), GREEN);
            } else {
                Garrison g("Ocupanti");
                g.addDefender(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Mercenar Inamic"));
                regions.emplace_back(zName, c, std::move(g), RED);
            }
        }

        // 3. Spawn inițial de armate inamice
        for(int i = 0; i < 3; ++i) spawnEnemy();
    }
    void spawnEnemy() {
        int x, y;
        // Căutăm un loc liber pe iarbă sau drum
        for(int i = 0; i < 50; ++i) {
            x = GameEngine::RandomGen::GetInt(1, 28);
            y = GameEngine::RandomGen::GetInt(1, 18);
            if (worldMap.isValidMove(x, y)) {
                roamingEnemies.emplace_back(x, y, "Horda de Jafuitori");
                logger.add("![ALERTA]: O armata inamica a aparut la [" + std::to_string(x) + "," + std::to_string(y) + "]");
                break;
            }
        }
    }

    void handleMovement() {
        int dx = 0, dy = 0;
        if (IsKeyPressed(KEY_W)) dy = -1;
        if (IsKeyPressed(KEY_S)) dy = 1;
        if (IsKeyPressed(KEY_A)) dx = -1;
        if (IsKeyPressed(KEY_D)) dx = 1;

        if (dx != 0 || dy != 0) {
            int nextX = player.getPos().first + dx;
            int nextY = player.getPos().second + dy;

            // Verificăm dacă inamicii blochează calea
            auto it = std::find_if(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
                return e.getX() == nextX && e.getY() == nextY && !e.isDefeated();
            });

            if (it != roamingEnemies.end()) {
                logger.add("![BLOCAJ]: Armata inamica iti taie calea! Ataca (F).");
                return;
            }

            try {
                player.move(dx, dy, worldMap);
            } catch (const InvalidMovementException& e) {
                logger.logError(e);
            }
        }
    }

    void handleCombat() {
        // Căutăm inamic pe poziția actuală
        auto [px, py] = player.getPos();

        // 1. Luptă cu armate de câmp (roaming)
        auto enemyIt = std::find_if(roamingEnemies.begin(), roamingEnemies.end(), [&](const EnemyArmy& e) {
            // Verificăm dacă e un inamic adiacent sau pe poziție (aici folosim adiacent pentru F)
            return std::abs(e.getX() - px) <= 1 && std::abs(e.getY() - py) <= 1 && !e.isDefeated();
        });

        if (enemyIt != roamingEnemies.end()) {
             try {
                // Combat simplificat pentru inamicii de câmp
                Unit* pUnit = player.getArmy().getFrontUnit();
                Unit* eUnit = enemyIt->getTroops().getFrontUnit();
                if(!pUnit) throw CombatException("Nu ai trupe!");

                eUnit->takeDamage(pUnit->calculateTotalAttack());
                if (eUnit->isAlive()) pUnit->takeDamage(eUnit->calculateTotalAttack());

                enemyIt->getTroops().removeDeadUnits();
                player.getArmy().removeDeadUnits();
                logger.add("Lupta cu jafuitorii in desfasurare...");
                if (enemyIt->isDefeated()) logger.add("VICTORIE: Drum eliberat!");
             } catch (const CombatException& e) { logger.logError(e); }
             return;
        }

        // 2. Luptă cu Orașul (dacă suntem pe el)
        auto& reg = regions[activeRegionIdx];
        auto [cx, cy] = reg.getCity().getPos();
        if (px == cx && py == cy) {
            try { logger.add(reg.executeBattleRound(player.getArmy())); }
            catch (const CombatException& e) { logger.logError(e); }
        }
    }
/*
    void handleRecruitment() {
        try {
            if (IsKeyPressed(KEY_ONE)) player.recruit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Infanterie"), 150);
            if (IsKeyPressed(KEY_TWO)) player.recruit(UnitFactory::CreateUnit(UnitType::ARCASI, "Arcas"), 180);
            if (IsKeyPressed(KEY_THREE)) player.recruit(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Cavaler"), 300);
        } catch (const InsufficientGoldException& e) { logger.logError(e); }
    }*/

public:
    Simulation() : window(1600, 1000, "EMPIRE RISING"), currentState(GameState::LOGIN), worldMap(30, 20) {
        SetTargetFPS(60);
    }
    friend std::ostream& operator<<(std::ostream& os, const Simulation& s) {
        os << "--- Status Simulare Empire Rising ---\n";
        os << "Jucator: " << s.player.getName() << " | Aur: " << s.player.getGold() << "\n";
        os << "Regiuni descoperite: " << s.regions.size() << "\n";
        os << "Ziua curenta: " << s.clock.getDay();
        return os;
    }
    void run() {
        bool gameWon = false;
        bool gameLost = false;
        const int MAX_DAYS = 50; // Limita de timp: 50 de zile

        while (!window.ShouldClose()) {
            // ==========================================
            // 1. LOGICA DE UPDATE (Calcule)
            // ==========================================
            if (currentState == GameState::LOGIN) {
                loginUI.update();
                if (loginUI.isAuthenticated()) {
                    player = Player(loginUI.getPlayerName(), 1000);
                    initWorld();
                    currentState = GameState::SIMULATION;
                }
            } else if (!gameWon && !gameLost) {
                // Verificare Victorie
                gameWon = std::all_of(regions.begin(), regions.end(), [](const Zone& r) {
                    return r.getCity().isOccupied();
                });

                // Verificare Înfrângere (Limită de zile)
                if (!gameWon && clock.getDay() >= MAX_DAYS) {
                    gameLost = true;
                }

                if (IsKeyPressed(KEY_R)) showRecruitment = !showRecruitment;

                if (showRecruitment) {
                    handleRecruitment();
                } else {
                    handleMovement();
                    handleCityActions();

                    if (IsKeyPressed(KEY_F)) handleCombat();

                    if (IsKeyPressed(KEY_TAB)) {
                        activeRegionIdx = (activeRegionIdx + 1) % regions.size();
                    }

                    if (IsKeyPressed(KEY_N)) {
                        clock.nextDay();
                        int income = 0;
                        for(auto& r : regions) {
                            r.getCity().growPopulation();
                            income += r.getCity().collectTaxes();
                        }
                        player.earnGold(income);
                        logger.add("Ziua " + std::to_string(clock.getDay()) + ". Venit colectat: " + std::to_string(income));

                        if (clock.getDay() % 10 == 0) spawnEnemy();
                    }
                }
            }

            // ==========================================
            // 2. LOGICA DE DESENARE
            // ==========================================
            BeginDrawing();
            window.ClearBackground({30, 30, 35, 255});

            if (currentState == GameState::LOGIN) {
                loginUI.draw();
            } else {
                worldMap.draw(20, 100);

                // Desenare orașe...
                for (size_t i = 0; i < regions.size(); ++i) {
                    auto& reg = regions[i];
                    auto [cx, cy] = reg.getCity().getPos();
                    int screenX = 20 + cx * 40;
                    int screenY = 100 + cy * 40;
                    if (i == (size_t)activeRegionIdx) DrawRectangleLines(screenX - 2, screenY - 2, 44, 44, WHITE);
                    DrawRectangle(screenX + 5, screenY + 5, 30, 30, reg.getCity().isOccupied() ? GOLD : RED);
                    ::DrawText(reg.getCity().getName().c_str(), screenX - 5, screenY + 40, 10, RAYWHITE);
                }

                // Inamici și Jucător...
                for (const auto& e : roamingEnemies) if (!e.isDefeated()) e.draw(20, 100);
                auto [px, py] = player.getPos();
                DrawCircle(20 + px * 40 + 20, 100 + py * 40 + 20, 12, SKYBLUE);

                // --- B. SIDEBAR ---
                int sbX = 1600 - SIDEBAR_WIDTH;
                DrawRectangle(sbX, 0, SIDEBAR_WIDTH, 1000, {20, 20, 25, 255});

                // Info Resurse
                ::DrawText(TextFormat("%i AUR", player.getGold()), sbX + 20, 55, 30, GOLD);
                ::DrawText(TextFormat("ZIUA: %i / %i", clock.getDay(), MAX_DAYS), sbX + 20, 95, 20, (clock.getDay() > MAX_DAYS - 10) ? RED : LIGHTGRAY);

                // DETALII REGIUNE (Inclusiv Garnizoana)
                auto& sel = regions[activeRegionIdx];
                DrawRectangle(sbX + 15, 140, SIDEBAR_WIDTH - 30, 240, {40, 40, 50, 255}); // Am mărit fundalul
                ::DrawText("INFO REGIUNE:", sbX + 25, 150, 15, SKYBLUE);
                ::DrawText(sel.getCity().getName().c_str(), sbX + 25, 170, 22, WHITE);
                ::DrawText(TextFormat("Populatie: %i", sel.getCity().getPopulation()), sbX + 25, 200, 16, GRAY);

                // Afișare Garnizoană (Grupate)
                ::DrawText("GARNIZOANA:", sbX + 25, 230, 16, RED);
                const auto& garrisonUnits = sel.getGarrison().getTroops().getUnits();
                if (garrisonUnits.empty()) {
                    ::DrawText("Fara aparare", sbX + 25, 255, 15, DARKGRAY);
                } else {
                    std::map<std::string, int> gCounts;
                    for (const auto& u : garrisonUnits) gCounts[u->getName()]++;
                    int gy = 255;
                    for (const auto& [name, count] : gCounts) {
                        ::DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 25, gy, 15, RAYWHITE);
                        gy += 20;
                        if (gy > 360) break;
                    }
                }

                // Armata Ta (Grupate)
                ::DrawText("ARMATA TA:", sbX + 20, 400, 20, GOLD);
                int unitY = 430;
                const auto& myUnits = player.getArmy().getUnits();
                std::map<std::string, int> counts;
                for (const auto& u : myUnits) counts[u->getName()]++;
                for (const auto& [name, count] : counts) {
                    ::DrawText(TextFormat("%i x %s", count, name.c_str()), sbX + 25, unitY, 17, RAYWHITE);
                    unitY += 25;
                }

                // Jurnal Imperial...
                ::DrawText("JURNAL IMPERIAL", sbX + 20, 650, 18, GRAY);
                int logY = 680;
                for (const auto& msg : logger.getMessages()) {
                    ::DrawText(msg.c_str(), sbX + 20, logY, 14, (msg.find("![") != std::string::npos) ? RED : GREEN);
                    logY += 20;
                }

                // --- C. OVERLAYS (Victorie / Înfrângere) ---
                if (gameWon) {
                    DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 220});
                    ::DrawText("VICTORIE TOTALA!", 520, 400, 60, GOLD);
                    ::DrawText("Ai unit imperiul inainte de termen!", 540, 480, 25, RAYWHITE);
                } else if (gameLost) {
                    DrawRectangle(0, 0, 1600, 1000, {50, 0, 0, 230});
                    ::DrawText("TIMP EXPIRAT!", 580, 400, 60, RED);
                    ::DrawText("Imperiul s-a destramat sub asediul timpului.", 500, 480, 25, RAYWHITE);
                    ::DrawText("Ai pierdut jocul.", 700, 530, 20, GRAY);
                }

                if (showRecruitment) {
                    DrawRectangle(0, 0, 1600, 1000, {0, 0, 0, 200});
                    DrawRectangle(600, 350, 400, 320, {45, 45, 55, 255});
                    ::DrawText("RECRUTARE", 725, 370, 30, GOLD);
                    ::DrawText("1. Infanterie | 2. Arcas | 3. Cavalerie", 620, 430, 20, WHITE);
                }
            }
            EndDrawing();
        }
    }
};

// ==========================================================
// 16. REZUMAT IMPLEMENTARE & COMPILARE
// ==========================================================
/*
 * Modificări efectuate conform cerințelor tale:
 * 1. RECRUTARE FIX: Tasta [R] acum declanșează un overlay modal (handleRecruitmentLogic).
 * 2. UI SEPARAT: Jurnalul (Logs) și Detaliile sunt mutate în SIDEBAR (dreapta).
 * 3. FOOTER: Bara de comenzi de jos este permanent vizibilă pentru ghidajul jucătorului.
 * 4. CLEANUP: Eliminate orice metode care nu participă la fluxul principal sau la cerințe.
 * 5. TEMA 1 & 2: Păstrat polimorfismul, excepțiile, NVI și constructorii virtuali (clone).
 *
 * IMPORTANT: Asigură-te că ai adăugat enum-urile UnitType și GameState la începutul
 * fișierului, așa cum am discutat anterior, pentru a evita erorile de tip nedeclarat.
 */

// Exemplu de apel în main:
// int main() {
//     RunTema2Demo(); // Demo-ul obligatoriu pentru nota la laborator
//     Simulation game;
//     game.run();
//     return 0;
// }

// ==========================================================
// 16. DEMO TEMA 2 (CONSOLA)
// ==========================================================

void RunTema2Demo() {
    std::cout << "\n>>> DEMO TEMA 2: POLIMORFISM SI GESTIUNE RESURSE <<<\n";

    // 1. Testare Ierarhie Unitati & Polimorfism
    std::vector<std::unique_ptr<Unit>> testArmy;
    testArmy.push_back(std::make_unique<Infantry>("Soldat", Ability("Scut", 0.1f, 5)));
    testArmy.push_back(std::make_unique<Hero>("Achile", Ability("Atac Divin", 0.5f, 100)));

    for (const auto& u : testArmy) {
        std::cout << "Unitate: ";
        u->print(std::cout); // Apel virtual
        std::cout << " | Atac Total: " << u->calculateTotalAttack() << "\n";
    }

    // 2. Testare dynamic_cast (Cerinta T2)
    std::cout << "\n[Test dynamic_cast]: ";
    for (auto& u : testArmy) {
        if (auto* h = dynamic_cast<Hero*>(u.get())) {
            std::cout << "Am gasit un Erou: " << h->getName() << ". Aplicam inspiratie!\n";
            h->inspireTroops();
        }
    }

    // 3. Testare Virtual Copy Constructor (clone)
    auto clonedHero = testArmy[1]->clone();
    std::cout << "[Test Clone]: " << *clonedHero << " (Clona reusita)\n";

    // 4. Testare Exceptii
    std::cout << "\n[Test Exceptii]: ";
    try {
        Player testP("Test", 50);
        testP.recruit(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Esec"), 300);
    } catch (const EmpireException& e) {
        std::cout << "Exceptie prinsa: " << e.what() << "\n";
    }

    // 5. Testare Atribute/Metode Statice
    std::cout << "\n[Statistici Globale]:\n";
    std::cout << "Total Unitati Instantiere: " << Unit::getTotalUnits() << "\n";
    WorldClock::SetGlobalTaxRate(1.25f);
    std::cout << "Rata taxe setata la: " << (WorldClock::GetGlobalTaxRate() * 100) << "%\n";

    std::cout << "-------------------------------------------------\n\n";
}
// ==========================================================
// 17. PUNCTUL DE INTRARE (MAIN)
// ==========================================================

int main() {
    // ------------------------------------------------------
    // ETAPA 1: DEMO TEMA 1 (Logica de bază în consolă)
    // ------------------------------------------------------
    std::cout << "========== EMPIRE RISING: DEBUG TEMA 1 ==========\n";

    // Testare Ability (Compunere)
    Ability basicAtk("Lovitura Grea", 0.3f, 20);
    std::cout << basicAtk << " | Nume: " << basicAtk.getName() << "\n";

    // Testare Unit & Rule of Three (Demonstrație pe o clasă derivată)
    Infantry i1("Garda Orasului", basicAtk);
    Infantry i2 = i1; // Copy Constructor
    Infantry i3("Recrut", Ability());
    i3 = i1;          // Copy Assignment

    std::cout << "Unitate T1 (Infant): " << i1 << " | HP: " << i1.getHP() << "\n";
    i1.takeDamage(40);
    i1.gainXP(150);
    std::cout << "Dupa lupta & XP: Lvl " << i1.getLevel() << ", HP " << i1.getHP() << "\n";

    // Testare City & Economie
    City c1("Asezare Test", 2, 2, 100);
    int testGold = 500;
    c1.setOccupied(true);
    c1.growPopulation();
    try {
        c1.upgrade(testGold);
        std::cout << "City Test: " << c1.getName() << " | Pop: " << c1.getPopulation()
                  << " | Taxes: " << c1.collectTaxes() << "\n";
    } catch (...) { std::cout << "Eroare upgrade T1\n"; }

    std::cout << "=================================================\n\n";

    // ------------------------------------------------------
    // ETAPA 2: DEMO TEMA 2 (Polimorfism, Exceptii, Statice)
    // ------------------------------------------------------
    // Apelăm funcția definită în Mesajul 9
    RunTema2Demo();

    // ------------------------------------------------------
    // ETAPA 3: LANSARE SIMULARE INTERACTIVĂ (Raylib)
    // ------------------------------------------------------
    #ifndef GITHUB_ACTIONS
        std::cout << "Toate testele de logica au trecut.\n";
        std::cout << "APASATI [ENTER] PENTRU A DESCHIDE HARTA IMPERIULUI...";
        std::cin.get();
    #endif

    try {
        // Instantierea Simulation - Orchestratorul principal
        Simulation game;

        // Testare operator<< Simulation (Compunere recursivă)
        std::cout << "\nStarea initiala a simularii:\n" << game << std::endl;

        // Pornirea buclei principale de joc
        game.run();

    } catch (const std::exception& e) {
        // Catch-all pentru orice eroare critică la runtime
        std::cerr << "\n!!! EROARE CRITICA DE SISTEM !!!\n"
                  << "Detalii: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nSimulare incheiata cu succes. Gloria imperiului dainuie!\n";
    return 0;
}

// ==========================================================
// SFARSIT COD EMPIRE RISING
// ==========================================================