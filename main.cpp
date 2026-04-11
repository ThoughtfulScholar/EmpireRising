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

namespace GameEngine {
    /**
     * @struct ColorPalette
     * Definirea culorilor pentru interfata fara a repeta codul.
     */
    struct ColorPalette {
        [[nodiscard]] static raylib::Color Gold()    { return {255, 203, 0, 255}; }
        [[nodiscard]] static raylib::Color Sky()     { return {102, 191, 255, 255}; }
        [[nodiscard]] static raylib::Color UIBack()  { return {20, 20, 25, 255}; }
        [[nodiscard]] static raylib::Color Victory() { return {0, 228, 48, 255}; }

        /**
         * NOTA: Aceasta functie va fi apelata in render() pentru a evita
         * warning-ul [unusedFunction] de la Cppcheck/Clang-Tidy.
         */
        [[nodiscard]] static raylib::Color Enemy()   { return {230, 41, 55, 255}; }
    };

    /**
     * @class RandomGen
     * Generator modern de numere aleatorii pentru a evita rand() limitat.
     */
    struct RandomGen {
        static float GetFloat(float min, float max) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }
    };

    /**
     * @class Logger
     * Gestioneaza mesajele afisate in consola din joc.
     */
    class Logger {
    private:
        std::vector<std::string> messages;
    public:
        void add(const std::string& msg) {
            messages.push_back(msg);
            // Mentinem ultimele 14 mesaje pentru lizibilitate
            if (messages.size() > 14) {
                messages.erase(messages.begin());
            }
        }
        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
        friend std::ostream& operator<<(std::ostream& os, const Logger& logger) {
            os << "--- Jurnal de Lupta ---\n";
            for (const auto& msg : logger.messages) {
                os << " >> " << msg << "\n";
            }
            return os;
        }
    };
}

// Definitii globale de stari si tipuri
enum class UnitType { INFANTERIE, ARCASI, CAVALERIE, GARDA_CETATE };
enum class GameState { LOGIN, SIMULATION, VICTORIE };

/**
 * @struct UnitClassData
 * Datele de baza pentru fiecare tip de unitate.
 */
struct UnitClassData {
    std::string className;
    int baseHp, baseAtk, baseDef, cost;
};

struct UnitRegistry {
    [[nodiscard]] static UnitClassData GetData(UnitType type) {
        switch (type) {
            case UnitType::INFANTERIE: return {"Infanterie Grea", 280, 45, 35, 120};
            case UnitType::ARCASI:     return {"Arcasi de Elita", 160, 75, 15, 150};
            case UnitType::CAVALERIE:  return {"Cavalerie Grea", 350, 65, 25, 250};
            default:                   return {"Recrut", 100, 20, 10, 50};
        }
    }
};
/**
 * @class Ability
 * Tehnica speciala folosita in timpul luptei.
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
    friend std::ostream& operator<<(std::ostream& os, const Ability& ab) {
        os << "[Abilitate: " << ab.name << " | Sansa: " << (ab.activationChance * 100) << "% | Bonus: +" << ab.bonusDamage << "]";
        return os;
    }
};

/**
 * @class Unit
 * Implementare conform Temei 1: Gestiunea resurselor prin Rule of Three.
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
    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unitate: " << u.name << " (Lvl " << u.level << ", HP: " << u.hp << "/" << u.maxHp << ") " << u.specialAbility;
        return os;
    }
    // Constructor de initializare
    Unit(std::string n, UnitType t, Ability ab)
        : name(std::move(n)), type(t), level(1), xp(0), specialAbility(std::move(ab)) {
        const auto data = UnitRegistry::GetData(t);
        hp = maxHp = data.baseHp;
        atk = data.baseAtk;
        def = data.baseDef;
    }

    // --- RULE OF THREE ---
    Unit(const Unit& other) 
        : name(other.name), 
          type(other.type), 
          hp(other.hp), 
          maxHp(other.maxHp), 
          atk(other.atk), 
          def(other.def), 
          level(other.level), 
          xp(other.xp), 
          specialAbility(other.specialAbility) {
    }

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

    ~Unit() {
        
    }

    // --- LOGICA DE LUPTA ---
    [[nodiscard]] int calculateTotalAttack() const {
        return atk + specialAbility.trigger() + (level * 5);
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
            maxHp += 45;
            hp = maxHp;
            atk += 8;
            def += 4;
        }
    }

    [[nodiscard]] bool isAlive() const { return hp > 0; }
    [[nodiscard]] int getHP() const { return hp; }

    /**
     * NOTA: Aceasta functie va fi apelata in render pentru a elimina
     * warning-ul [unusedFunction] si pentru a afisa HP-ul maxim.
     */
    [[nodiscard]] int getMaxHP() const { return maxHp; }

    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
};
/**
 * @class City
 * Gestioneaza economia, populatia si nivelul de dezvoltare al unei asezari.
 */
class City {
private:
    std::string cityName;
    int cityLevel;
    int baseIncome;
    int population; // ADAUGAT: Membru nou pentru populatie
    bool occupied;

public:
    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "Oras: " << c.cityName << " (Nivel: " << c.cityLevel << ", Pop: " << c.population << ", Ocupat: " << (c.occupied ? "DA" : "NU") << ")";
        return os;
    }
    explicit City(std::string name = "Asezare", int income = 150)
        : cityName(std::move(name)), cityLevel(1), baseIncome(income),
          population(100), occupied(false) {}

    /**
     * @brief Cresterea nivelului orasului folosind aurul furnizat.
     */
    bool upgrade(int& playerGold) {
        if (!occupied) {
            return false;
        }
        const int cost = getUpgradeCost();
        if (playerGold >= cost) {
            playerGold -= cost;
            cityLevel++;
            population += 75; // Populatia creste semnificativ la upgrade
            return true;
        }
        return false;
    }

    /**
     * @brief Simuleaza cresterea demografica organica (apelata la Zi Noua).
     */
    void growPopulation() {
        if (occupied) {
            population += 5 + (cityLevel * 2);
        }
    }

    [[nodiscard]] int getUpgradeCost() const {
        // Costul depinde acum si de marimea populatiei care trebuie administrata
        return 350 * cityLevel + (cityLevel * cityLevel * 50) + (population / 5);
    }

    [[nodiscard]] int collectTaxes() const {
        if (!occupied) return 0;
        // Taxele depind acum de nivelul orasului PLUS numarul de cetateni
        return baseIncome + (cityLevel * 100) + (population / 2);
    }

    void setOccupied(bool status) { occupied = status; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    [[nodiscard]] const std::string& getName() const { return cityName; }
    [[nodiscard]] int getLevel() const { return cityLevel; }
    [[nodiscard]] int getPopulation() const { return population; }
};

/**
 * @class Garrison
 * Reprezinta oastea inamica ce apara o cetate.
 */
class Garrison {
private:
    std::vector<Unit> soldiers;
    std::string faction;

public:
    explicit Garrison(std::string fName = "Aparatori") : faction(std::move(fName)) {}
    friend std::ostream& operator<<(std::ostream& os, const Garrison& g) {
        os << "Garnizoana: " << g.faction << " (" << g.soldiers.size() << " unitati)";
        return os;
    }
    void addDefender(const Unit& u) {
        soldiers.push_back(u);
    }

    void cleanup() {
        std::erase_if(soldiers, [](const auto& u) { return !u.isAlive(); });
    }

    [[nodiscard]] bool isDefeated() const {
        return soldiers.empty();
    }

    [[nodiscard]] Unit* getFrontUnit() {
        if (!soldiers.empty()) return &soldiers[0];
        return nullptr;
    }

    [[nodiscard]] const std::vector<Unit>& getSoldiers() const { return soldiers; }
};

/**
 * @class Player
 * Reprezinta entitatea jucatorului, tezaurul si armata acestuia.
 */
class Player {
private:
    std::string commanderName;
    int gold;
    std::vector<Unit> army;

public:
    explicit Player(std::string name = "Comandant", int startingGold = 1000)
        : commanderName(std::move(name)), gold(startingGold) {}
    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Comandant: " << p.commanderName << " | Tezaur: " << p.gold << " aur | Trupe: " << p.army.size();
        return os;
    }
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

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return commanderName; }
    [[nodiscard]] std::vector<Unit>& getArmy() { return army; }
};
/**
 * @class Zone
 * Reprezinta o regiune ce contine o cetate si o garnizoana inamica.
 */
class Zone {
private:
    std::string zoneName;
    City localCity;
    Garrison enemyGarrison;
    raylib::Color mapTint;

public:
    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "--- Zona: " << z.zoneName << " ---\n  " << z.localCity << "\n  " << z.enemyGarrison;
        return os;
    }
    Zone(std::string name, City city, Garrison garrison, raylib::Color tint)
        : zoneName(std::move(name)), localCity(std::move(city)),
          enemyGarrison(std::move(garrison)), mapTint(tint) {}

    [[nodiscard]] std::string executeBattleRound(Player& player) {
        if (enemyGarrison.isDefeated()) {
            localCity.setOccupied(true);
            return "Cetatea " + localCity.getName() + " este deja ocupata.";
        }

        if (player.getArmy().empty()) {
            return "Nu mai ai unitati pentru a lupta!";
        }

        auto& pUnit = player.getArmy().front();
        auto* eUnit = enemyGarrison.getFrontUnit();

        int pAtk = pUnit.calculateTotalAttack();
        eUnit->takeDamage(pAtk);
        std::string result = pUnit.getName() + " loveste cu " + std::to_string(pAtk) + ". ";

        if (!eUnit->isAlive()) {
            enemyGarrison.cleanup();
            pUnit.gainXP(80);
            if (enemyGarrison.isDefeated()) {
                localCity.setOccupied(true);
                return result + "Garnizoana a fost infranta!";
            }
            return result + "Inamic eliminat.";
        }

        int eAtk = eUnit->calculateTotalAttack();
        pUnit.takeDamage(eAtk);
        result += "Inamicul riposteaza: -" + std::to_string(eAtk) + " HP.";

        if (!pUnit.isAlive()) {
            player.updateArmyStatus();
            return result + " Ai pierdut o unitate!";
        }

        return result;
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
struct RecruitmentCenter {
    static void Hire(Player& p, UnitType type, GameEngine::Logger& logger) {
        auto data = UnitRegistry::GetData(type);
        if (p.spendGold(data.cost)) {
            Ability ab;
            if (type == UnitType::INFANTERIE) ab = Ability("Zid Scuturi", 0.3f, 15);
            else if (type == UnitType::ARCASI) ab = Ability("Sageata Foc", 0.4f, 25);
            else ab = Ability("Sarja", 0.25f, 45);

            p.recruitUnit(Unit(data.className, type, ab));
            logger.add("Recrutat: " + data.className);
        } else {
            logger.add("Aur insuficient!");
        }
    }
};

/**
 * @class LoginManager
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

    // FIX Clang-Tidy: Return by const reference
    [[nodiscard]] const std::string& getPlayerName() const { return inputBuffer; }
    friend std::ostream& operator<<(std::ostream& os, const LoginManager& lm) {
        os << "[Sistem Login] Status: " << (lm.authenticated ? "Autentificat" : "In asteptare")
           << " | Input actual: " << (lm.inputBuffer.empty() ? "(gol)" : lm.inputBuffer);
        return os;
    }
};
/**
 * @class Simulation
 * Coordoneaza fluxul principal al jocului, randarea si interactiunea.
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
        // Regiunea 1
        Garrison g1("Garda de Fier");
        g1.addDefender(Unit("Infanterist Veteran", UnitType::INFANTERIE, Ability("Blocaj", 0.2f, 10)));
        g1.addDefender(Unit("Arcas Cetate", UnitType::ARCASI, Ability("Foc", 0.15f, 20)));

        // Regiunea 2
        Garrison g2("Legiunea Neagra");
        g2.addDefender(Unit("Cavaler Greu", UnitType::CAVALERIE, Ability("Sarja", 0.3f, 40)));

        // --- ORASE NOI ADAUGATE ---

        // Regiunea 3 - Arhipelagul de Sud
        Garrison g3("Piratii de Smarald");
        g3.addDefender(Unit("Arcas Maritim", UnitType::ARCASI, Ability("Sageata Otravita", 0.35f, 15)));
        g3.addDefender(Unit("Infanterie Usoara", UnitType::INFANTERIE, Ability("Evadare", 0.2f, 5)));

        // Regiunea 4 - Citadela Inhetata
        Garrison g4("Garda Inhetata");
        g4.addDefender(Unit("Garda de Elita", UnitType::INFANTERIE, Ability("Inghet", 0.4f, 25)));
        g4.addDefender(Unit("Cavaler de Nord", UnitType::CAVALERIE, Ability("Avalansa", 0.3f, 50)));

        // Popularea hartii
        worldMap.emplace_back("Valea Sperantei", City("Veridonia", 200), g1, GREEN);
        worldMap.emplace_back("Muntii de Fier", City("Ironhold", 350), g2, DARKGRAY);
        worldMap.emplace_back("Coasta de Azur", City("Port Royal", 280), g3, BLUE);
        worldMap.emplace_back("Tundra Oarba", City("Frosthelm", 450), g4, SKYBLUE);
    }

    void handleInput() {
        if (IsKeyPressed(KEY_ESCAPE)) window.Close();

        if (currentState == GameState::LOGIN) {
            loginUI.update();
            if (loginUI.isAuthenticated()) {
                player = Player(loginUI.getPlayerName(), 1500);
                player.recruitUnit(Unit("Garda Personala", UnitType::INFANTERIE, Ability("Protectie", 0.2f, 15)));
                currentState = GameState::SIMULATION;
            }
            return;
        }

        if (currentState == GameState::VICTORIE) return;

        if (IsKeyPressed(KEY_TAB)) {
            activeZoneIdx = (activeZoneIdx + 1) % static_cast<int>(worldMap.size());
        }

        if (IsKeyPressed(KEY_R)) showRecruitment = !showRecruitment;

        auto& currentZone = worldMap[static_cast<size_t>(activeZoneIdx)];

        if (IsKeyPressed(KEY_F)) {
            logger.add(currentZone.executeBattleRound(player));
            if (checkVictory()) currentState = GameState::VICTORIE;
        }

        if (IsKeyPressed(KEY_U)) {
            auto& currentCity = currentZone.getCity(); // Luăm referință la oraș

            if (!currentCity.isOccupied()) {
                logger.add("Eroare: Nu poti upgrada un oras inamic!");
            } else {
                int currentGold = player.getGold();
                if (currentCity.upgrade(currentGold)) {
                    player.spendGold(player.getGold() - currentGold); // Sincronizăm aurul
                    logger.add("Upgrade: " + currentCity.getName() + " la Nivel " + std::to_string(currentCity.getLevel()));
                } else {
                    logger.add("Fonduri insuficiente pentru upgrade!");
                }
            }
        }

        if (IsKeyPressed(KEY_N)) {
            gameDay++;
            int taxes = 0;
            for (auto& z : worldMap) {
                z.getCity().growPopulation(); // ADAUGAT: Creste populatia zilnic
                taxes += z.getCity().collectTaxes();
            }
            player.earnGold(taxes);
            logger.add("Ziua " + std::to_string(gameDay) + ": Taxe (" + std::to_string(taxes) + " aur)");
        }

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
            raylib::Text::Draw("Toate cetatile au fost cucerite.", 620, 600, 25, WHITE);
        }
        else {
            // --- UI HEADER ---
            DrawRectangle(0, 0, 1600, 140, GameEngine::ColorPalette::UIBack());
            raylib::Text::Draw("COMANDANT: " + player.getName(), 50, 40, 40, WHITE);
            raylib::Text::Draw("TEZAUR: " + std::to_string(player.getGold()) + " AUR", 50, 90, 25, GameEngine::ColorPalette::Gold());
            raylib::Text::Draw("ZIUA: " + std::to_string(gameDay), 1400, 40, 35, LIGHTGRAY);

            // --- CENTRU: DETALII REGIUNE SI ORAS ---
            const auto& zone = worldMap[static_cast<size_t>(activeZoneIdx)];
            const auto& city = zone.getCity();

            DrawRectangle(50, 160, 950, 500, zone.getTint().Alpha(0.2f));
            raylib::Text::Draw("REGIUNE: " + zone.getName(), 80, 190, 45, BLACK);

            // Caseta Detalii Oras (Populatie si Cost Upgrade)
            DrawRectangle(80, 250, 450, 80, raylib::Color::LightGray().Alpha(0.3f));
            raylib::Text::Draw("ORAS: " + city.getName(), 95, 260, 20, DARKGRAY);
            raylib::Text::Draw("POPULATIE: " + std::to_string(city.getPopulation()) + " cetateni", 95, 282, 22, DARKBLUE);

            if (city.isOccupied()) {
                std::string costMsg = "COST UPGRADE: " + std::to_string(city.getUpgradeCost()) + " AUR";
                raylib::Text::Draw(costMsg, 95, 305, 18, MAROON);
                raylib::Text::Draw("STARE: CETATE CONTROLATA (NIVEL " + std::to_string(city.getLevel()) + ")", 80, 350, 25, GREEN);
            } else {
                // Utilizam GameEngine::ColorPalette::Enemy() aici pentru a elimina warning-ul [unusedFunction]
                raylib::Text::Draw("STARE: SUB CONTROL INAMIC", 80, 350, 25, GameEngine::ColorPalette::Enemy());

                // Afisare Garnizoana Inamica
                int ey = 400;
                for (const auto& e : zone.getGarrison().getSoldiers()) {
                    raylib::Text::Draw("- " + e.getName() + " (" + std::to_string(e.getHP()) + " HP)", 100, ey, 20, DARKGRAY);
                    ey += 30;
                }
            }

            // --- DREAPTA: MENIU RECRUTARE ---
            if (showRecruitment) {
                DrawRectangle(1050, 160, 500, 420, raylib::Color::RayWhite().Alpha(0.9f));
                DrawRectangleLines(1050, 160, 500, 420, DARKBLUE);
                raylib::Text::Draw("RECRUTARE", 1080, 180, 30, DARKBLUE);
                raylib::Text::Draw("1. Infanterie (120g)\n2. Arcasi (150g)\n3. Cavalerie (250g)", 1080, 240, 22, BLACK);
                raylib::Text::Draw("Apasa cifra corespunzatoare", 1080, 530, 16, DARKGRAY);
            }

            // --- STANGA-JOS: ARMATA JUCATOR ---
            int py = 720;
            raylib::Text::Draw("ARMATA TA:", 50, 680, 25, DARKBLUE);
            for (const auto& u : player.getArmy()) {
                DrawRectangle(50, py, 450, 45, GameEngine::ColorPalette::Sky());

                // Utilizam u.getMaxHP() aici pentru a elimina warning-ul [unusedFunction]
                std::string unitStats = u.getName() + " [Lvl " + std::to_string(u.getLevel()) +
                                       "] HP: " + std::to_string(u.getHP()) + "/" + std::to_string(u.getMaxHP());

                raylib::Text::Draw(unitStats, 70, py + 12, 18, WHITE);
                py += 55;
                if (py > 1100) break;
            }

            // --- DREAPTA-JOS: LOGGER ---
            DrawRectangle(1050, 680, 500, 440, BLACK);
            int ly = 700;
            for (const auto& m : logger.getMessages()) {
                raylib::Text::Draw(">> " + m, 1070, ly, 18, GREEN);
                ly += 25;
            }

            // --- FOOTER ---
            DrawRectangle(0, 1150, 1600, 50, GameEngine::ColorPalette::UIBack());
            raylib::Text::Draw("[N] Zi Noua | [F] Lupta | [U] Upgrade Oras | [R] Recrutare | [TAB] Regiuni", 350, 1165, 20, WHITE);
        }
        EndDrawing();
    }

public:
    Simulation()
        : window(1600, 1200, "EMPIRE RISING"),
          currentState(GameState::LOGIN)
    {
        SetTargetFPS(60);
        initWorld();
    }

    void run() {
        while (!window.ShouldClose()) {
            handleInput();
            render();
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const Simulation& sim) {
        os << "======= STATUS SIMULARE =======\n"
           << " Ziua curenta: " << sim.gameDay << "\n"
           << " " << sim.player << "\n" // Compunere: apelează operator<< de la Player
           << " Zone active in lume: " << sim.worldMap.size() << "\n"
           << " " << sim.logger << "\n"; // Compunere: apelează operator<< de la Logger
        os << "===============================";
        return os;
    }
};

int main() {
    // ==========================================================
    // SECTIUNE OBLIGATORIE: DEMO TEMA 1 (CONSOLA)
    // Se apeleaza TOATE functiile membre publice conform cerintei.
    // ==========================================================
    std::cout << "========== EMPIRE RISING: DEBUG TEMA 1 ==========\n";

    // 1. Testare Ability
    Ability fireStrike("Atac de Foc", 0.4f, 30);
    std::cout << fireStrike << "\n";
    std::cout << "Metode Ability: Name=" << fireStrike.getName()
              << ", Trigger=" << fireStrike.trigger() << "\n";

    // 2. Testare Unit + RULE OF THREE
    Unit u1("Garda Imperiala", UnitType::INFANTERIE, fireStrike);
    Unit u2 = u1; // Copy Ctor
    Unit u3("Recrut", UnitType::ARCASI, Ability());
    u3 = u1;      // Copy Assignment

    int damagePotential = u1.calculateTotalAttack();
    std::cout << damagePotential << "\n";
    u1.takeDamage(50);
    u1.gainXP(120);
    std::cout << "Metode Unit: HP=" << u1.getHP() << "/" << u1.getMaxHP()
              << ", Lvl=" << u1.getLevel() << ", Viu=" << u1.isAlive() << "\n";

    // 3. Testare City
    City testCity("Veridonia", 200);
    int gold = 1000;
    testCity.setOccupied(true);
    testCity.growPopulation();
    testCity.upgrade(gold);
    std::cout << "Metode City: Name=" << testCity.getName()
              << ", Lvl=" << testCity.getLevel()
              << ", Pop=" << testCity.getPopulation()
              << ", CostUp=" << testCity.getUpgradeCost()
              << ", Taxes=" << testCity.collectTaxes() << ", Ocupat=" << testCity.isOccupied() << "\n";

    // 4. Testare Garrison
    Garrison g1("Legiunea Neagra");
    g1.addDefender(u1);
    g1.cleanup();
    std::cout << "Metode Garrison: Defeated=" << g1.isDefeated()
              << ", SoldiersCount=" << g1.getSoldiers().size() << "\n";
    if(g1.getFrontUnit()) std::cout << "Front Unit: " << g1.getFrontUnit()->getName() << "\n";

    // 5. Testare Player
    Player p("Cezar", 2000);
    p.recruitUnit(u2);
    p.earnGold(500);
    p.spendGold(100);
    p.updateArmyStatus();
    std::cout << "Metode Player: Name=" << p.getName()
              << ", Gold=" << p.getGold()
              << ", ArmySize=" << p.getArmy().size()
              << ", Prima unitate=" << (p.getArmy().empty() ? "Niciuna" : p.getArmy()[0].getName()) << "\n";

    // 6. Testare Zone (Compunere & Getters)
    Zone z1("Valea Sperantei", testCity, g1, raylib::Color::Green());
    std::cout << z1 << "\n";
    std::cout << "Metoda complexa Zone (Battle): " << z1.executeBattleRound(p) << "\n";
    std::cout << "Acces prin Zone: Oras=" << z1.getCity().getName()
              << ", Inamici=" << z1.getGarrison().getSoldiers().size() << "\n"
                << ", Culoare R=" << (int)z1.getTint().r << "\n";;
    //std::cout << "Tint Map: R=" << (int)z1.getTint().r << "\n";
    z1.getCity().setOccupied(false);

    // 7. Testare Utilitare (RecruitmentCenter & RandomGen & Logger)
    GameEngine::Logger log;
    log.add("Test log");
    RecruitmentCenter::Hire(p, UnitType::ARCASI, log);
    std::cout << log << "\n";
    std::cout << "Numar mesaje in log: " << log.getMessages().size() << "\n";
    std::cout << "Random Test: " << GameEngine::RandomGen::GetFloat(0, 1) << "\n";


    // 8. Testare LoginManager
    LoginManager lm;
    std::cout << lm << "\n";
    std::cout << "Login Debug: Autentificat=" << lm.isAuthenticated()
              << ", Buffer=" << lm.getPlayerName() << "\n";
    if (false) { lm.update(); lm.draw(); } // Apel tehnic pentru functii grafice

    std::cout << "=================================================\n";

    #ifndef GITHUB_ACTIONS
        std::cout << "APASATI ENTER PENTRU A PORNI JOCUL (Raylib)...";
        std::cin.get();
    #endif

    // ==========================================================
    // PORNIRE JOC INTERACTIV (RAYLIB)
    // ==========================================================
    try {
        Simulation game;
        std::cout << game << std::endl; // Test operator<< Simulation
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Eroare critica: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
