#include "raylib-cpp.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <random>
#include <ranges>
#include <cmath>
#include <ostream>

namespace GameEngine {
    /**
     * @struct ColorPalette
     * Paleta de culori a interfetei.
     */
    struct ColorPalette {
        [[nodiscard]] static raylib::Color Gold()    { return {255, 203, 0, 255}; }
        [[nodiscard]] static raylib::Color Sky()     { return {102, 191, 255, 255}; }
        [[nodiscard]] static raylib::Color UIBack()  { return {20, 20, 25, 255}; }
        [[nodiscard]] static raylib::Color Victory() { return {0, 228, 48, 255}; }
        [[nodiscard]] static raylib::Color Enemy()   { return {230, 41, 55, 255}; }
    };

    /**
     * @class RandomGen
     * Generator de numere pentru sanse de activare.
     */
    class RandomGen {
    public:
        static float GetFloat(float min, float max) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }
    };

    /**
     * @class Logger
     * Inregistreaza fluxul de evenimente.
     */
    class Logger {
    private:
        std::vector<std::string> messages;
    public:
        void add(const std::string& msg) {
            messages.push_back(msg);
            if (messages.size() > 14) messages.erase(messages.begin());
        }
        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
    };
}

enum class UnitType { INFANTERIE, ARCASI, CAVALERIE, GARDA_CETATE };
enum class GameState { LOGIN, SIMULATION, VICTORIE };
struct UnitClassData {
    std::string className;
    int baseHp, baseAtk, baseDef, cost;

    friend std::ostream& operator<<(std::ostream& os, const UnitClassData& d) {
        os << d.className << " (Cost: " << d.cost << ")";
        return os;
    }
};

class UnitRegistry {
public:
    [[nodiscard]] static UnitClassData GetData(UnitType type) {
        switch (type) {
            case UnitType::INFANTERIE: return {"Infanterie", 280, 45, 35, 120};
            case UnitType::ARCASI:     return {"Arcasi", 160, 75, 15, 150};
            case UnitType::CAVALERIE:  return {"Cavalerie", 350, 65, 25, 250};
            default:                   return {"Garda", 300, 50, 40, 200};
        }
    }
};

/**
 * @class Ability
 * Reprezinta o tehnica de lupta folosita in momente critice.
 */
class Ability {
private:
    std::string name;
    float activationChance;
    int bonusDamage;

public:
    explicit Ability(std::string n = "Atac standard", float chance = 0.0f, int bonus = 0)
        : name(std::move(n)), activationChance(chance), bonusDamage(bonus) {}

    [[nodiscard]] int trigger() const {
        if (GameEngine::RandomGen::GetFloat(0.0f, 1.0f) <= activationChance) {
            return bonusDamage;
        }
        return 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& a) {
        os << "<< " << a.name << " >>";
        return os;
    }
};

/**
 * @class Unit
 * Entitatea de baza a jocului. Implementeaza Rule of Three conform Temei 1.
 */
class Unit {
private:
    std::string name;
    UnitType type;
    int hp;
    int maxHp;
    int atk;
    int def;
    int level;
    int xp;
    Ability specialAbility;

public:
    Unit(std::string n, UnitType t, Ability ab)
        : name(std::move(n)), type(t), hp(0), maxHp(0), atk(0), def(0),
          level(1), xp(0), specialAbility(std::move(ab)) {
        const auto data = UnitRegistry::GetData(t);
        hp = data.baseHp;
        maxHp = data.baseHp;
        atk = data.baseAtk;
        def = data.baseDef;
    }

    // --- RULE OF THREE ---
    Unit(const Unit& other) = default;
    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            type = other.type;
            hp = other.hp;
            maxHp = other.maxHp;
            atk = other.atk;
            def = other.def;
            level = other.level;
            xp = other.xp;
            specialAbility = other.specialAbility;
        }
        return *this;
    }
    ~Unit() = default;

    [[nodiscard]] int calculateTotalAttack() const {
        return atk + specialAbility.trigger() + (level * 7);
    }

    [[nodiscard]] int getBaseAttackPower() const {
        return atk + (level * 7);
    }

    void takeDamage(int rawDamage) {
        const int finalDamage = std::max(10, rawDamage - def);
        hp = std::max(0, hp - finalDamage);
    }

    void gainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp = 0;
            maxHp += 40;
            hp = maxHp;
            atk += 15;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Soldat: " << u.name << " (Lvl " << u.level << ") " << u.specialAbility;
        return os;
    }

    [[nodiscard]] bool isAlive() const { return hp > 0; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
};
/**
 * @class City
 * Gestioneaza economia, populatia si nivelul de dezvoltare.
 */
class City {
private:
    std::string cityName;
    int cityLevel;
    int baseIncome;
    int population;
    bool occupied;

public:
    explicit City(std::string name = "Asezare", int income = 150)
        : cityName(std::move(name)), cityLevel(1), baseIncome(income),
          population(100), occupied(false) {}

    /**
     * @brief Calculeaza taxele bazate pe nivel si populatie.
     */
    [[nodiscard]] int collectTaxes() const {
        if (!occupied) return 0;
        float efficiency = 1.0f + (static_cast<float>(cityLevel) * 0.2f);
        return static_cast<int>((baseIncome + (population / 2)) * efficiency);
    }

    [[nodiscard]] int getUpgradeCost() const {
        return 300 * cityLevel + (cityLevel * cityLevel * 45) + (population / 5);
    }

    /**
     * @brief Upgrade-ul creste nivelul si ofera un boom demografic.
     */
    void upgrade() {
        cityLevel++;
        population += 50 * cityLevel; // Bonus de populatie la upgrade
    }

    /**
     * @brief Simuleaza cresterea organica a populatiei.
     */
    void simulateGrowth() {
        if (occupied) {
            population += 5 + (cityLevel * 2); // Crestere zilnica
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "Cetatea << " << c.cityName << " >> [Nivel: " << c.cityLevel
           << " | Populatie: " << c.population << "]";
        return os;
    }

    void setOccupied(bool status) { occupied = status; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    [[nodiscard]] const std::string& getName() const { return cityName; }
    [[nodiscard]] int getLevel() const { return cityLevel; }
    [[nodiscard]] int getPopulation() const { return population; }
};

/**
 * @class Garrison
 * Reprezinta apararea unei locatii. Compunere cu clasa Unit.
 */
class Garrison {
private:
    std::vector<Unit> soldiers;
    std::string faction;

public:
    explicit Garrison(std::string fName = "Garda") : faction(std::move(fName)) {}

    void addDefender(const Unit& u) {
        soldiers.push_back(u);
    }

    void cleanup() {
        std::erase_if(soldiers, [](const auto& u) { return !u.isAlive(); });
    }

    [[nodiscard]] bool isDefeated() const { return soldiers.empty(); }

    [[nodiscard]] Unit* getFrontUnit() {
        return soldiers.empty() ? nullptr : &soldiers[0];
    }

    friend std::ostream& operator<<(std::ostream& os, const Garrison& g) {
        os << "Garnizoana " << g.faction << " cu " << g.soldiers.size() << " trupe";
        return os;
    }

    [[nodiscard]] const std::vector<Unit>& getSoldiers() const { return soldiers; }
};
/**
 * @class Player
 * Reprezinta utilizatorul si resursele sale.
 */
class Player {
private:
    std::string commanderName;
    int gold;
    std::vector<Unit> army;

public:
    explicit Player(std::string name = "Comandant", int startingGold = 1000)
        : commanderName(std::move(name)), gold(startingGold) {}

    void recruitUnit(const Unit& u) {
        army.push_back(u);
    }

    void updateArmyStatus() {
        std::erase_if(army, [](const auto& u) { return !u.isAlive(); });
    }

    bool spendGold(int amount) {
        if (gold >= amount) {
            gold -= amount;
            return true;
        }
        return false;
    }

    void earnGold(int amount) { gold += amount; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Comandantul << " << p.commanderName << " >> (Aur: " << p.gold << ")";
        return os;
    }

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return commanderName; }
    [[nodiscard]] std::vector<Unit>& getArmy() { return army; }
};

/**
 * @class Zone
 * Reprezinta o regiune strategica. Exemplu de COMPUNERE (City + Garrison).
 */
class Zone {
private:
    std::string zoneName;
    City localCity;
    Garrison enemyGarrison;
    raylib::Color mapTint;

public:
    Zone(std::string name, City city, Garrison garrison, raylib::Color tint)
        : zoneName(std::move(name)), localCity(std::move(city)),
          enemyGarrison(std::move(garrison)), mapTint(tint) {}

    [[nodiscard]] std::string executeBattleRound(Player& player) {
        if (enemyGarrison.isDefeated()) {
            localCity.setOccupied(true);
            return "Zona << " + zoneName + " >> este sub controlul tau.";
        }

        if (player.getArmy().empty()) return "Infrangere: Armata ta a fost distrusa!";

        auto& pUnit = player.getArmy().front();
        auto* eUnit = enemyGarrison.getFrontUnit();

        // Atacul jucatorului
        int pAtk = pUnit.calculateTotalAttack();
        eUnit->takeDamage(pAtk);
        std::string log = pUnit.getName() + " loveste cu " + std::to_string(pAtk) + ". ";

        if (!eUnit->isAlive()) {
            enemyGarrison.cleanup();
            pUnit.gainXP(80);
            if (enemyGarrison.isDefeated()) {
                localCity.setOccupied(true);
                return log + "Cetatea a fost cucerita!";
            }
            return log + "Inamic eliminat.";
        }

        // Riposta inamicului
        int eAtk = eUnit->calculateTotalAttack();
        pUnit.takeDamage(eAtk);
        player.updateArmyStatus();
        return log + "Inamicul riposteaza cu " + std::to_string(eAtk) + ".";
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "REGIUNE: << " << z.zoneName << " >> | " << z.localCity << " | " << z.enemyGarrison;
        return os;
    }

    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const City& getCity() const { return localCity; }
    [[nodiscard]] const Garrison& getGarrison() const { return enemyGarrison; }
    [[nodiscard]] raylib::Color getTint() const { return mapTint; }
};

/**
 * @class RecruitmentCenter
 */
class RecruitmentCenter {
public:
    static void Hire(Player& p, UnitType type, GameEngine::Logger& logger) {
        auto data = UnitRegistry::GetData(type);
        if (p.spendGold(data.cost)) {
            Ability ab;
            if (type == UnitType::INFANTERIE) ab = Ability("Zid de Scuturi", 0.3f, 15);
            else if (type == UnitType::ARCASI) ab = Ability("Sageata de Foc", 0.4f, 25);
            else ab = Ability("Sarja", 0.25f, 40);

            p.recruitUnit(Unit(data.className, type, ab));
            logger.add("Unitate noua: " + data.className);
        } else {
            logger.add("Fonduri insuficiente!");
        }
    }
};
/**
 * @class LoginManager
 * Gestioneaza ecranul de autentificare. Optimizat pentru a evita copii inutile.
 */
class LoginManager {
private:
    std::string inputBuffer;
    bool authenticated = false;

public:
    void update() {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (inputBuffer.length() < 12)) {
                inputBuffer += static_cast<char>(key);
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer.empty()) inputBuffer.pop_back();
        if (IsKeyPressed(KEY_ENTER) && !inputBuffer.empty()) authenticated = true;
    }

    void draw() const {
        DrawRectangleGradientV(0, 0, 1600, 1200, DARKGRAY, BLACK);
        raylib::Text::Draw("EMPIRE RISING", 550, 400, 70, GameEngine::ColorPalette::Gold());
        raylib::Text::Draw("Numele Comandantului:", 650, 500, 25, LIGHTGRAY);
        DrawRectangle(600, 550, 400, 60, RAYWHITE);
        raylib::Text::Draw(inputBuffer, 620, 562, 35, DARKGRAY);
    }

    [[nodiscard]] bool isAuthenticated() const { return authenticated; }

    /**
     * @brief Rezolvare eroare [returnByReference].
     * Returneaza referinta constanta pentru a evita copierea std::string.
     */
    [[nodiscard]] const std::string& getPlayerName() const { return inputBuffer; }
};

/**
 * @class Simulation
 * Coordoneaza fluxul principal al jocului. Exemplu de compunere masiva.
 */
class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    LoginManager loginUI;
    Player player;
    GameEngine::Logger logger;
    std::vector<Zone> worldMap;

    int activeZoneIdx = 0;
    int gameDay = 1;
    bool showRecruitment = false;

    [[nodiscard]] bool checkVictory() const {
        return std::ranges::all_of(worldMap, [](const auto& z) {
            return z.getCity().isOccupied();
        });
    }

    void initWorld() {
        Garrison g1("Garda de Fier");
        g1.addDefender(Unit("Infanterist", UnitType::INFANTERIE, Ability("Blocaj", 0.2f, 10)));

        Garrison g2("Legiunea Neagra");
        g2.addDefender(Unit("Cavaler", UnitType::CAVALERIE, Ability("Sarja", 0.3f, 40)));

        worldMap.emplace_back("Valea Sperantei", City("Veridonia", 200), g1, GREEN);
        worldMap.emplace_back("Muntii de Fier", City("Ironhold", 350), g2, DARKGRAY);
    }

public:
    Simulation()
        : window(1600, 1200, "EMPIRE RISING"),
          currentState(GameState::LOGIN)
    {
        SetTargetFPS(60);
        initWorld();
    }
    void handleInput() {
        if (currentState == GameState::LOGIN) {
            loginUI.update();
            if (loginUI.isAuthenticated()) {
                // Transferam numele prin valoare o singura data pentru a initia jucatorul
                player = Player(loginUI.getPlayerName(), 1500);
                currentState = GameState::SIMULATION;
            }
            return;
        }

        // Navigare si UI
        if (IsKeyPressed(KEY_TAB)) activeZoneIdx = (activeZoneIdx + 1) % static_cast<int>(worldMap.size());
        if (IsKeyPressed(KEY_R)) showRecruitment = !showRecruitment;

        auto& currentZone = worldMap[static_cast<size_t>(activeZoneIdx)];

        // Logica de Upgrade: Creste nivelul si populatia
        if (IsKeyPressed(KEY_U) && currentZone.getCity().isOccupied()) {
            int cost = currentZone.getCity().getUpgradeCost();
            if (player.spendGold(cost)) {
                currentZone.getCity().upgrade();
                logger.add("Upgrade reusit: " + currentZone.getCity().getName() + " (Nivel " + std::to_string(currentZone.getCity().getLevel()) + ")");
            } else {
                logger.add("Fonduri insuficiente! Necesari: " + std::to_string(cost) + " aur.");
            }
        }

        // Logica de Lupta
        if (IsKeyPressed(KEY_F) && !currentZone.getCity().isOccupied()) {
            logger.add(currentZone.executeBattleRound(player));
            if (checkVictory()) currentState = GameState::VICTORIE;
        }

        // Logica Zi Noua: Colectare taxe si cresterea populatiei
        if (IsKeyPressed(KEY_N)) {
            gameDay++;
            int taxesTotal = 0;
            for (auto& z : worldMap) {
                z.getCity().simulateGrowth(); // Populatia creste organic
                taxesTotal += z.getCity().collectTaxes();
            }
            player.earnGold(taxesTotal);
            logger.add("Ziua " + std::to_string(gameDay) + ": Taxe incasate (+" + std::to_string(taxesTotal) + " aur).");
        }

        // Recrutare
        if (showRecruitment) {
            if (IsKeyPressed(KEY_ONE)) RecruitmentCenter::Hire(player, UnitType::INFANTERIE, logger);
            if (IsKeyPressed(KEY_TWO)) RecruitmentCenter::Hire(player, UnitType::ARCASI, logger);
            if (IsKeyPressed(KEY_THREE)) RecruitmentCenter::Hire(player, UnitType::CAVALERIE, logger);
        }
    }

    void render() {
        BeginDrawing();
        window.ClearBackground(RAYWHITE);

        if (currentState == GameState::LOGIN) {
            loginUI.draw();
        }
        else if (currentState == GameState::VICTORIE) {
            DrawRectangle(0, 0, 1600, 1200, BLACK);
            raylib::Text::Draw("VICTORIE TOTALA!", 500, 500, 80, GameEngine::ColorPalette::Victory());
            raylib::Text::Draw("Toate cetatile au fost cucerite sub comanda ta.", 550, 600, 25, WHITE);
        }
        else if (currentState == GameState::SIMULATION) {
            const auto& zone = worldMap[static_cast<size_t>(activeZoneIdx)];
            const auto& city = zone.getCity();

            // --- HEADER ---
            DrawRectangle(0, 0, 1600, 140, GameEngine::ColorPalette::UIBack());
            raylib::Text::Draw("COMANDANT: " + player.getName(), 50, 40, 35, WHITE);
            raylib::Text::Draw("TEZAUR: " + std::to_string(player.getGold()) + " AUR", 50, 90, 25, GameEngine::ColorPalette::Gold());
            raylib::Text::Draw("ZIUA: " + std::to_string(gameDay), 1400, 40, 30, LIGHTGRAY);

            // --- PANOU CENTRAL ---
            DrawRectangle(50, 160, 950, 500, zone.getTint().Alpha(0.15f));
            raylib::Text::Draw("REGIUNEA: << " + zone.getName() + " >>", 80, 190, 45, BLACK);

            if (!city.isOccupied()) {
                raylib::Text::Draw("AMENINTARE PREZENTA:", 80, 320, 25, GameEngine::ColorPalette::Enemy());
                int ey = 360;
                for (const auto& e : zone.getGarrison().getSoldiers()) {
                    std::string enemyStr = "- " + e.getName() + " | HP: " + std::to_string(e.getHP()) + " | Forta: " + std::to_string(e.getBaseAttackPower());
                    raylib::Text::Draw(enemyStr, 100, ey, 20, DARKGRAY);
                    ey += 30;
                }
            } else {
                raylib::Text::Draw("STARE: CETATE CONTROLATA", 80, 260, 25, DARKGREEN);
                raylib::Text::Draw("Populatie actuala: " + std::to_string(city.getPopulation()) + " cetateni", 80, 310, 22, DARKGRAY);
                raylib::Text::Draw("Venit estimat: +" + std::to_string(city.collectTaxes()) + " aur/zi", 80, 345, 22, DARKGRAY);

                int cost = city.getUpgradeCost();
                raylib::Color upgColor = (player.getGold() >= cost) ? DARKGREEN : MAROON;
                raylib::Text::Draw("Cost Upgrade [U]: " + std::to_string(cost) + " Aur (Nivel " + std::to_string(city.getLevel()) + ")", 80, 400, 25, upgColor);
            }

            // --- RECRUTARE & TRUPE ---
            if (showRecruitment) {
                DrawRectangle(1050, 160, 500, 420, raylib::Color::RayWhite().Alpha(0.9f));
                DrawRectangleLines(1050, 160, 500, 420, DARKBLUE);
                raylib::Text::Draw("CENTRUL DE RECRUTARE", 1080, 180, 28, DARKBLUE);
                raylib::Text::Draw("1. Infanterie (120g)\n2. Arcasi (150g)\n3. Cavalerie (250g)", 1080, 240, 22, BLACK);
            }

            // --- ARMATA ---
            raylib::Text::Draw("TRUPELE TALE:", 50, 680, 25, DARKBLUE);
            int py = 720;
            for (const auto& u : player.getArmy()) {
                DrawRectangle(50, py, 450, 45, GameEngine::ColorPalette::Sky());
                std::string unitStats = u.getName() + " [Lvl " + std::to_string(u.getLevel()) + "] HP: " + std::to_string(u.getHP()) + "/" + std::to_string(u.getMaxHP());
                raylib::Text::Draw(unitStats, 70, py + 12, 18, WHITE);
                py += 55; if (py > 1100) break;
            }

            // --- LOGGER ---
            DrawRectangle(1050, 680, 500, 440, BLACK);
            int ly = 700;
            for (const auto& m : logger.getMessages()) {
                raylib::Text::Draw(">> " + m, 1070, ly, 18, GREEN);
                ly += 25;
            }

            // --- FOOTER ---
            DrawRectangle(0, 1150, 1600, 50, GameEngine::ColorPalette::UIBack());
            raylib::Text::Draw("[N] Zi Noua (Taxe) | [F] Lupta | [U] Upgrade | [R] Meniu Recrutare | [TAB] Schimba Zona", 280, 1165, 20, WHITE);
        }
        EndDrawing();
    }

    void run() { while (!window.ShouldClose()) { handleInput(); render(); } }
};
/**
 * @brief Punctul de intrare in aplicatie.
 * Contine teste pentru functionalitatile cerute la Tema 1:
 * - Operatori supraîncărcați (<<)
 * - Rule of Three (prin copierea de unitati)
 * - Compunere (Zone -> City, Garrison)
 */
int main() {
    // 1. TESTARE OPERATORI ȘI RULE OF THREE (Consolă)
    std::cout << "--- EMPIRE RISING: TESTARE CERINTE TEMA 1 ---" << std::endl;

    // Test Rule of Three (Copy Constructor & Assignment)
    Ability ab("Sarja", 0.5f, 30);
    Unit u1("Garda Imperiala", UnitType::CAVALERIE, ab);
    Unit u2 = u1; // Copy Constructor
    Unit u3("Infanterie", UnitType::INFANTERIE, Ability());
    u3 = u2;      // Copy Assignment

    std::cout << "Original: " << u1 << std::endl;
    std::cout << "Copie:    " << u3 << std::endl;

    // Test Compunere si Operatori Afisare
    City c1("Veridonia", 250);
    c1.setOccupied(true);
    Garrison g1("Legiunea a IV-a");
    g1.addDefender(u1);

    Zone z1("Valea Sperantei", c1, g1, GREEN);
    std::cout << "Test Compunere Zone: " << z1 << std::endl;

    std::cout << "\n--- LANSARE INTERFATA GRAFICA ---" << std::endl;

    // 2. LANSARE SIMULARE
    try {
        Simulation game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Eroare critica in executie: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}