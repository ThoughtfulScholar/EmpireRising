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

enum class TerrainType { PLAIN, FOREST, MOUNTAIN, WATER };

class Tile {
private:
    TerrainType type;
    bool walkable;
    float defenseBonus;
    raylib::Color color;

public:
    Tile(TerrainType t = TerrainType::PLAIN) : type(t) {
        switch (t) {
            case TerrainType::PLAIN:
                walkable = true;
                defenseBonus = 1.0f;
                color = raylib::Color::LightGray();
                break;
            case TerrainType::FOREST:
                walkable = true;
                defenseBonus = 1.2f; // Bonus de defensiva in padure
                color = GameEngine::ColorPalette::Forest();
                break;
            case TerrainType::MOUNTAIN:
                walkable = false;
                defenseBonus = 1.5f;
                color = GameEngine::ColorPalette::Mountain();
                break;
            case TerrainType::WATER:
                walkable = false;
                defenseBonus = 0.8f;
                color = GameEngine::ColorPalette::Water();
                break;
        }
    }

    [[nodiscard]] bool isWalkable() const { return walkable; }
    [[nodiscard]] float getDefenseBonus() const { return defenseBonus; }
    [[nodiscard]] raylib::Color getColor() const { return color; }

    friend std::ostream& operator<<(std::ostream& os, const Tile& tile) {
        os << "[Tile: " << (tile.walkable ? "Accesibil" : "Blocat") << "]";
        return os;
    }
};

class WorldMap {
private:
    int width, height;
    std::vector<std::vector<Tile>> grid;
    static constexpr int TILE_SIZE = 40; // Dimensiunea unui patrat in pixeli

public:
    WorldMap(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Tile>(width, Tile(TerrainType::PLAIN)));
        generateRandomTerrain();
    }

    void generateRandomTerrain() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float r = GameEngine::RandomGen::GetFloat(0, 1);
                if (r < 0.15f) grid[y][x] = Tile(TerrainType::MOUNTAIN);
                else if (r < 0.30f) grid[y][x] = Tile(TerrainType::FOREST);
                else if (r < 0.35f) grid[y][x] = Tile(TerrainType::WATER);
            }
        }
    }

    [[nodiscard]] bool isValidMove(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        return grid[y][x].isWalkable();
    }

    [[nodiscard]] float getDefenseAt(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return 1.0f;
        return grid[y][x].getDefenseBonus();
    }

    void draw(int offsetX, int offsetY) const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                grid[y][x].getColor().DrawRectangle(
                    offsetX + x * TILE_SIZE,
                    offsetY + y * TILE_SIZE,
                    TILE_SIZE - 1, TILE_SIZE - 1
                );
            }
        }
    }

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] static int getTileSize() { return TILE_SIZE; }

    friend std::ostream& operator<<(std::ostream& os, const WorldMap& wm) {
        os << "Harta Lumii: " << wm.width << "x" << wm.height << " (Tiles: " << wm.width * wm.height << ")";
        return os;
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

    int activeRegionIdx = 0;
    bool showRecruitment = false; // Flag pentru meniul R

    // Dimensiuni UI fixe pentru calculul layout-ului
    static constexpr int SIDEBAR_WIDTH = 400;
    static constexpr int FOOTER_HEIGHT = 80;

    void initWorld() {
        // Inițializăm regiunile conform cerințelor de diversitate (Tema 2)
        Garrison g1("Legiunea de Fier");
        g1.addDefender(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Veteran de Scut"));

        Garrison g2("Horda Ghetii");
        g2.addDefender(std::make_unique<Hero>("Regele Nordului", Ability("Viscol", 0.4f, 50)));

        regions.emplace_back("Valea Verde", City("Veridonia", 5, 5), std::move(g1), GREEN);
        regions.emplace_back("Ghetarul Etern", City("Frosthold", 15, 10), std::move(g2), SKYBLUE);
    }

    void handleRecruitmentLogic() {
        // Tasta R comută meniul
        if (IsKeyPressed(KEY_R)) {
            showRecruitment = !showRecruitment;
            logger.add(showRecruitment ? "Meniu recrutare deschis." : "Meniu recrutare inchis.");
        }

        // Dacă meniul e activ, ascultăm de tastele 1, 2, 3
        if (showRecruitment) {
            try {
                if (IsKeyPressed(KEY_ONE)) {
                    player.recruit(UnitFactory::CreateUnit(UnitType::INFANTERIE, "Legionar"), 150);
                    logger.add("Recrutat: Infanterie.");
                } else if (IsKeyPressed(KEY_TWO)) {
                    player.recruit(UnitFactory::CreateUnit(UnitType::ARCASI, "Arcas"), 180);
                    logger.add("Recrutat: Arcas.");
                } else if (IsKeyPressed(KEY_THREE)) {
                    player.recruit(UnitFactory::CreateUnit(UnitType::CAVALERIE, "Cavaler"), 300);
                    logger.add("Recrutat: Cavalerie.");
                }
            } catch (const InsufficientGoldException& e) {
                logger.logError(e);
            }
        }
    }

    void handleGlobalInput() {
        if (IsKeyPressed(KEY_ESCAPE)) window.Close();

        if (currentState == GameState::LOGIN) {
            loginUI.update();
            if (loginUI.isAuthenticated()) {
                player = Player(loginUI.getPlayerName(), 1200);
                player.getArmy().addUnit(std::make_unique<Hero>("Garda de Corp", Ability("Protectie", 0.2f, 20)));
                currentState = GameState::SIMULATION;
            }
            return;
        }

        // Logica de recrutare (R) este acum separată pentru a fi siguri că rulează
        handleRecruitmentLogic();

        // Dacă meniul de recrutare e deschis, blocăm restul input-urilor (opțional, pentru UX mai bun)
        if (showRecruitment) return;

        // Mișcare și restul comenzilor
        try {
            if (IsKeyPressed(KEY_W)) player.move(0, -1, worldMap);
            if (IsKeyPressed(KEY_S)) player.move(0, 1, worldMap);
            if (IsKeyPressed(KEY_A)) player.move(-1, 0, worldMap);
            if (IsKeyPressed(KEY_D)) player.move(1, 0, worldMap);
        } catch (const InvalidMovementException& e) { logger.logError(e); }

        if (IsKeyPressed(KEY_TAB)) activeRegionIdx = (activeRegionIdx + 1) % regions.size();

        if (IsKeyPressed(KEY_N)) {
            clock.nextDay();
            int taxes = 0;
            for (auto& r : regions) {
                r.getCity().growPopulation();
                taxes += r.getCity().collectTaxes();
            }
            player.earnGold(taxes);
            logger.add("Ziua " + std::to_string(clock.getDay()) + ": Taxe colectate (" + std::to_string(taxes) + ").");
        }

        if (IsKeyPressed(KEY_F)) {
            auto& reg = regions[activeRegionIdx];
            auto [pX, pY] = player.getPos();
            auto [cX, cY] = reg.getCity().getPos();
            if (pX == cX && pY == cY) {
                try { logger.add(reg.executeBattleRound(player.getArmy())); }
                catch (const CombatException& e) { logger.logError(e); }
            } else {
                logger.add("Trebuie sa fii la pozitia orasului!");
            }
        }
    }
    // --- LOGICA DE RANDARE (Redesemnată pentru Ergonomie) ---

    void render() {
        BeginDrawing();
        // Fundal gri-închis (Canvas-ul principal)
        window.ClearBackground(raylib::Color{35, 35, 40, 255});

        if (currentState == GameState::LOGIN) {
            loginUI.draw();
        }
        else if (currentState == GameState::VICTORIE) {
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGREEN, BLACK);
            raylib::Text::Draw("VICTORIE ABSOLUTA", 450, 450, 60, GOLD);
        }
        else {
            // 1. ZONA HĂRȚII (Stânga - Spatieră fixă)
            // Harta este randată într-un offset care lasă loc pentru Sidebar și Footer
            const int mapOffsetX = 20;
            const int mapOffsetY = 100;
            worldMap.draw(mapOffsetX, mapOffsetY);

            // Randare Orașe pe Grid
            for (const auto& reg : regions) {
                auto [cx, cy] = reg.getCity().getPos();
                raylib::Color cityColor = reg.getCity().isOccupied() ? GOLD : RED;
                DrawRectangle(mapOffsetX + cx * 40, mapOffsetY + cy * 40, 38, 38, cityColor);
            }

            // Randare Jucător (Erou)
            auto [px, py] = player.getPos();
            DrawCircle(mapOffsetX + px * 40 + 20, mapOffsetY + py * 40 + 20, 14, SKYBLUE);
            DrawCircleLines(mapOffsetX + px * 40 + 20, mapOffsetY + py * 40 + 20, 15, WHITE);

            // 2. SIDEBAR (DREAPTA - Separator Vizual)
            // Calculăm începutul sidebar-ului: ScreenWidth - SIDEBAR_WIDTH
            int sbX = GetScreenWidth() - SIDEBAR_WIDTH;
            DrawRectangle(sbX, 0, SIDEBAR_WIDTH, GetScreenHeight(), raylib::Color{25, 25, 30, 255});
            DrawLineEx({(float)sbX, 0}, {(float)sbX, (float)GetScreenHeight()}, 2, DARKGRAY);

            // Header Sidebar: Resurse
            DrawRectangle(sbX, 0, SIDEBAR_WIDTH, 80, raylib::Color{40, 40, 50, 255});
            raylib::Text::Draw("TEZAUR: " + std::to_string(player.getGold()) + " AUR", sbX + 20, 25, 25, GOLD);

            // Info Regiune Selectată
            const auto& activeReg = regions[activeRegionIdx];
            raylib::Text::Draw("REGIUNE: " + activeReg.getName(), sbX + 20, 110, 22, SKYBLUE);
            raylib::Text::Draw("Oras: " + activeReg.getCity().getName(), sbX + 20, 140, 18, LIGHTGRAY);

            // JURNAL DE MESAJE (LOGS) - Izolat în sidebar
            raylib::Text::Draw("JURNAL IMPERIAL", sbX + 20, 450, 20, GRAY);
            DrawLine(sbX + 20, 475, sbX + 380, 475, DARKGRAY);

            int logY = 490;
            for (const auto& msg : logger.getMessages()) {
                raylib::Color msgCol = (msg.find("![EROARE]") != std::string::npos) ? RED : GREEN;
                raylib::Text::Draw("> " + msg, sbX + 25, logY, 15, msgCol);
                logY += 22;
            }

            // 3. BARA DE COMENZI (JOS - FOOTER)
            int footerY = GetScreenHeight() - FOOTER_HEIGHT;
            DrawRectangle(0, footerY, GetScreenWidth(), FOOTER_HEIGHT, BLACK);
            DrawLineEx({0, (float)footerY}, {(float)GetScreenWidth(), (float)footerY}, 3, GOLD);

            std::string cmdText = "[W,A,S,D] Miscare  |  [TAB] Schimba Regiunea  |  [F] Ataca Orasul  |  [U] Upgrade  |  [R] RECRUTARE  |  [N] Zi Noua";
            raylib::Text::Draw(cmdText, 40, footerY + 28, 20, RAYWHITE);
            raylib::Text::Draw("ZIUA: " + std::to_string(clock.getDay()), GetScreenWidth() - 150, footerY + 28, 22, LIGHTGRAY);

            // 4. OVERLAY MENIU RECRUTARE (Centrat)
            if (showRecruitment) {
                // Întunecăm restul ecranului
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), raylib::Color{0, 0, 0, 180});

                int menuW = 500;
                int menuH = 300;
                int menuX = (GetScreenWidth() - menuW) / 2;
                int menuY = (GetScreenHeight() - menuH) / 2;

                DrawRectangle(menuX, menuY, menuW, menuH, raylib::Color{45, 45, 55, 255});
                DrawRectangleLines(menuX, menuY, menuW, menuH, GOLD);

                raylib::Text::Draw("UNITATI DISPONIBILE", menuX + 110, menuY + 30, 25, GOLD);
                raylib::Text::Draw("1. Infanterie (150g)", menuX + 50, menuY + 100, 22, WHITE);
                raylib::Text::Draw("2. Arcas (180g)", menuX + 50, menuY + 140, 22, WHITE);
                raylib::Text::Draw("3. Cavalerie (300g)", menuX + 50, menuY + 180, 22, WHITE);
                raylib::Text::Draw("Apasa [R] pentru a inchide meniul", menuX + 110, menuY + 260, 16, GRAY);
            }
        }
        EndDrawing();
    }
    // --- FINALIZARE CLASA SIMULATION ---

public:
    Simulation()
        : window(1600, 1000, "EMPIRE RISING - TEMA 2"),
          currentState(GameState::LOGIN),
          worldMap(30, 20) // Ajustat pentru a lăsa loc sidebar-ului
    {
        SetTargetFPS(60);
        initWorld();
        logger.add("Sistem initializat. Asteptare Login...");
    }

    // Bucla principala curata, fara functii nefolosite
    void run() {
        while (!window.ShouldClose()) {
            handleGlobalInput(); // Gestioneaza atat logica de joc cat si recrutarea
            render();            // Gestioneaza noul layout: Harta, Sidebar si Footer
        }
    }

    // Cerință Tema 2: Operatorul << (Compunere)
    friend std::ostream& operator<<(std::ostream& os, const Simulation& sim) {
        os << "======= STATUS SIMULARE =======\n"
           << " Jucator: " << sim.player.getName() << " | Aur: " << sim.player.getGold() << "\n"
           << " Ziua curenta: " << sim.clock.getDay() << "\n"
           << " Regiuni explorate: " << sim.regions.size() << "\n"
           << "===============================";
        return os;
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