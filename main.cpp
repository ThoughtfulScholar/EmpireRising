#include "raylib-cpp.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <ctime>
#include <cmath>
#include <ostream>

/**
 * PROIECT: EmpireRising
 * CERINȚA: Tema 1 - Gestiune resurse, Unități, Luptă și Economie.
 */

namespace GameEngine {
    // Culori personalizate folosind cast-uri C++ pentru a evita avertismentele Clang-Tidy
    struct ColorPalette {
        [[nodiscard]] static raylib::Color Gold()    { return raylib::Color(255, 203, 0, 255); }
        [[nodiscard]] static raylib::Color Crimson() { return raylib::Color(190, 33, 55, 255); }
        [[nodiscard]] static raylib::Color Sky()     { return raylib::Color(102, 191, 255, 255); }
        [[nodiscard]] static raylib::Color Forest()  { return raylib::Color(0, 117, 44, 255); }
        [[nodiscard]] static raylib::Color UIBack()  { return raylib::Color(30, 30, 30, 255); }
        [[nodiscard]] static raylib::Color Victory() { return raylib::Color(0, 228, 48, 255); }
    };

    /**
     * @class Logger
     * Gestionează mesajele de sistem pentru jurnalul de luptă.
     */
    class Logger {
    private:
        std::vector<std::string> messages;
        const size_t maxMessages = 15; // Mărit pentru ecranul 1600x1200
    public:
        Logger() = default;

        void add(const std::string& m) {
            messages.push_back(m);
            if (messages.size() > maxMessages) {
                messages.erase(messages.begin());
            }
        }

        [[nodiscard]] const std::vector<std::string>& getMessages() const { return messages; }
        void clear() { messages.clear(); }

        // Cerință Tema 1: Operator<< pentru Logger
        friend std::ostream& operator<<(std::ostream& os, const Logger& logger) {
            os << "--- Jurnal Imperiu ---\n";
            for (const auto& msg : logger.messages) {
                os << ">> " << msg << "\n";
            }
            return os;
        }
    };
}

enum class GameState { LOGIN, SIMULATION, SUMMARY, VICTORY };

/**
 * @class LoginSystem
 * Interfață pentru introducerea numelui.
 */
class LoginSystem {
private:
    std::string playerName;
    int letterCount = 0;
    bool finished = false;
    const int maxLetters = 15;

public:
    LoginSystem() : playerName("") {}

    void update() {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (letterCount < maxLetters)) {
                playerName += static_cast<char>(key);
                letterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
            playerName.pop_back();
            letterCount--;
        }

        if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
            finished = true;
        }
    }

    void draw() const {
        // Rezoluție 1600x1200
        DrawRectangleGradientV(0, 0, 1600, 1200, DARKGRAY, BLACK);

        raylib::Text::Draw("EMPIRE RISING: HEGEMONY", 450, 300, 60, GameEngine::ColorPalette::Gold());
        raylib::Text::Draw("Introdu numele tau, Comandante:", 550, 450, 25, LIGHTGRAY);

        DrawRectangle(500, 500, 600, 80, RAYWHITE);
        DrawRectangleLines(500, 500, 600, 80, GameEngine::ColorPalette::Sky());

        raylib::Text::Draw(playerName, 530, 520, 45, DARKGRAY);

        if (letterCount > 0) {
            float blink = std::sin(static_cast<float>(GetTime()) * 5.0f);
            if (blink > 0.0f) {
                raylib::Text::Draw("APASA [ENTER] PENTRU A INCEPE CUCERIREA", 540, 620, 25, MAROON);
            }
        }
    }

    [[nodiscard]] bool isFinished() const { return finished; }
    [[nodiscard]] std::string getName() const { return playerName.empty() ? "Anonim" : playerName; }
};
enum class UnitType { INFANTERIE, ARCASI, CAVALERIE };

class Ability {
private:
    std::string name;
    float activationChance;
    int extraDamage;

public:
    explicit Ability(std::string n = "Atac standard", float chance = 0.0f, int dmg = 0)
        : name(std::move(n)), activationChance(chance), extraDamage(dmg) {}

    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        if (roll <= activationChance) return extraDamage;
        return 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] float getChance() const { return activationChance; }
    [[nodiscard]] int getBonus() const { return extraDamage; }

    friend std::ostream& operator<<(std::ostream& os, const Ability& ab) {
        os << ab.name << " (Sansa: " << ab.activationChance * 100 << "%)";
        return os;
    }
};

// Corecție: Definirea structurii lipsă pentru datele claselor
struct UnitClassData {
    std::string className;
    int baseHp;
    int baseAtk;
    int baseDef;
    int cost;
    std::string trait;
};

class UnitRegistry {
public:
    [[nodiscard]] static UnitClassData GetData(UnitType type) {
        switch (type) {
            case UnitType::INFANTERIE:
                return {"Infanterie Greas", 200, 30, 25, 100, "Bonus: Rezistenta (+5 Def)"};
            case UnitType::ARCASI:
                return {"Arcasi de Elita", 120, 55, 10, 130, "Bonus: Lovitura Critica"};
            case UnitType::CAVALERIE:
                return {"Cavalerie Grea", 250, 45, 20, 190, "Bonus: Impact devastator"};
            default:
                return {"Recrut", 100, 15, 10, 50, "Niciun bonus"};
        }
    }
};
class Unit {
private:
    std::string name;
    UnitType type;
    int hp, maxHp;
    int atk, def;
    int level;
    int xp;
    Ability specialAbility;

public:
    // Constructor de inițializare
    Unit(std::string n, UnitType t, Ability ab)
        : name(std::move(n)), type(t), level(1), xp(0), specialAbility(std::move(ab)) {
        UnitClassData data = UnitRegistry::GetData(t);
        hp = maxHp = data.baseHp;
        atk = data.baseAtk;
        def = data.baseDef;
    }

    // Cerință Tema 1: Constructor de copiere
    Unit(const Unit& other)
        : name(other.name), type(other.type), hp(other.hp), maxHp(other.maxHp),
          atk(other.atk), def(other.def), level(other.level), xp(other.xp),
          specialAbility(other.specialAbility) {}

    // Cerință Tema 1: Operator= de copiere
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

    // Destructor
    ~Unit() = default;

    [[nodiscard]] int getAttackPower() const {
        int bonus = specialAbility.trigger();
        int classBonus = (type == UnitType::CAVALERIE) ? (maxHp / 8) : 0;
        return atk + bonus + classBonus;
    }

    void receiveDamage(int amount) {
        int effectiveDef = (type == UnitType::INFANTERIE) ? (def + 8) : def;
        int finalDmg = std::max(10, amount - effectiveDef);
        hp = std::max(0, hp - finalDmg);
    }

    void gainExperience(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            maxHp += 40;
            hp = maxHp;
            atk += 10;
            def += 5;
        }
    }

    [[nodiscard]] bool isAlive() const { return hp > 0; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getLvl() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << u.name << " [Lvl " << u.level << "] HP: " << u.hp << "/" << u.maxHp;
        return os;
    }
};
/**
 * @class Item
 * Reprezintă un obiect de echipament ce oferă bonusuri.
 */
class Item {
private:
    std::string name;
    int atkBonus;
    int defBonus;
    int price;

public:
    explicit Item(std::string n = "Obiect Inutil", int a = 0, int d = 0, int p = 0)
        : name(std::move(n)), atkBonus(a), defBonus(d), price(p) {}

    [[nodiscard]] std::string getInfo() const {
        return name + " (+" + std::to_string(atkBonus) + " Atk, +" + std::to_string(defBonus) + " Def)";
    }

    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Item& it) {
        os << it.name << " [" << it.price << "g]";
        return os;
    }
};

/**
 * @class Vault
 * Sistem de stocare securizat pentru artefacte.
 */
class Vault {
private:
    std::vector<Item> storage;
    size_t capacity;

public:
    explicit Vault(size_t cap = 8) : capacity(cap) {}

    bool addItem(const Item& it) {
        if (storage.size() < capacity) {
            storage.push_back(it);
            return true;
        }
        return false;
    }

    [[nodiscard]] size_t getCount() const { return storage.size(); }
    [[nodiscard]] const std::vector<Item>& getItems() const { return storage; }

    friend std::ostream& operator<<(std::ostream& os, const Vault& v) {
        os << "Vault (" << v.storage.size() << "/" << v.capacity << " slots filled)";
        return os;
    }
};

/**
 * @class City
 * Gestionează dezvoltarea urbană și veniturile fiscale.
 */
class City {
private:
    std::string name;
    int level;
    int baseTax;
    bool occupied;

public:
    explicit City(std::string n = "Asezare", int tax = 75)
        : name(std::move(n)), level(1), baseTax(tax), occupied(false) {}

    void upgrade() { level++; }

    [[nodiscard]] int calculateIncome() const {
        if (!occupied) return 0;
        return baseTax + (level * 40) + (level * level * 10);
    }

    [[nodiscard]] int getUpgradeCost() const { return 200 * level; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    void setOccupied(bool status) { occupied = status; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const City& c) {
        os << "Oras: " << c.name << " [Lvl " << c.level << "] - "
           << (c.occupied ? "Cucerit" : "Liber");
        return os;
    }
};
class Terrain {
private:
    std::string typeName;
    float atkMod;
    float defMod;
    raylib::Color mapColor;

public:
    explicit Terrain(std::string name = "Campie", float atk = 1.0f, float def = 1.0f, raylib::Color col = GRAY)
        : typeName(std::move(name)), atkMod(atk), defMod(def), mapColor(col) {}

    [[nodiscard]] float getAtkMod() const { return atkMod; }
    [[nodiscard]] float getDefMod() const { return defMod; }
    [[nodiscard]] const raylib::Color& getColor() const { return mapColor; }
    [[nodiscard]] const std::string& getName() const { return typeName; }

    friend std::ostream& operator<<(std::ostream& os, const Terrain& t) {
        os << "Teren: " << t.typeName;
        return os;
    }
};

class Zone {
private:
    std::string zoneName;
    Terrain env;
    City localCity;
    std::vector<Unit> enemies;

public:
    Zone(std::string name, Terrain t, City c)
        : zoneName(std::move(name)), env(std::move(t)), localCity(std::move(c)) {}

    void spawnEnemy(const Unit& u) { enemies.push_back(u); }

    std::string battleRound(class Player& p);

    // Versiunea non-const (permite modificarea orașului, ex: city.setOccupied)
    [[nodiscard]] City& getCity() { return localCity; }

    // Versiunea const (permite doar citirea, rezolvă eroarea din checkVictory)
    [[nodiscard]] const City& getCity() const { return localCity; }

    [[nodiscard]] const Terrain& getTerrain() const { return env; }
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] size_t getEnemyCount() const { return enemies.size(); }

    // Versiunea const pentru inamici (bună pentru randare)
    [[nodiscard]] const std::vector<Unit>& getEnemies() const { return enemies; }
    // Versiunea non-const (pentru erase)
    [[nodiscard]] std::vector<Unit>& getEnemies() { return enemies; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zona: " << z.zoneName << " | Inamici: " << z.enemies.size();
        return os;
    }
};
class Quest {
private:
    std::string title;
    std::string goal;
    int reward;
    bool completed;

public:
    Quest(std::string t, std::string g, int r)
        : title(std::move(t)), goal(std::move(g)), reward(r), completed(false) {}

    // Doar declarăm metoda, o implementăm mai jos
    void checkStatus(class Player& p, bool condition);

    [[nodiscard]] bool isDone() const { return completed; }
    [[nodiscard]] const std::string& getTitle() const { return title; }
    [[nodiscard]] const std::string& getGoal() const { return goal; }
    [[nodiscard]] int getReward() const { return reward; }
};

class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> army;
    Vault treasury;

public:
    explicit Player(std::string n = "Comandant", int initialGold = 600)
        : name(std::move(n)), gold(initialGold), treasury(8) {}

    void addGold(int amount) { gold += amount; }
    bool spendGold(int amount) {
        if (gold >= amount) { gold -= amount; return true; }
        return false;
    }

    void recruit(const Unit& u) { army.push_back(u); }

    void removeDeadUnits() {
        army.erase(std::remove_if(army.begin(), army.end(),
            [](const Unit& u) { return !u.isAlive(); }), army.end());
    }

    int collectTaxes(std::vector<Zone>& worldZones) {
        int total = 0;
        for (auto& zone : worldZones) {
            if (zone.getCity().isOccupied()) {
                total += zone.getCity().calculateIncome();
            }
        }
        gold += total;
        return total;
    }

    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] std::vector<Unit>& getArmy() { return army; }
    [[nodiscard]] Vault& getVault() { return treasury; }
};
void Quest::checkStatus(Player& p, bool condition) {
    if (!completed && condition) {
        completed = true;
        p.addGold(reward);
    }
}
// Implementare battleRound (acum că Player este definit)
std::string Zone::battleRound(Player& p) {
    if (enemies.empty()) return "Zona este libera! Orasul " + localCity.getName() + " iti apartine.";
    if (p.getArmy().empty()) return "Infrangere! Nu mai ai unitati de lupta!";

    Unit& myUnit = p.getArmy()[0];
    Unit& enemyUnit = enemies[0];

    int pAtk = static_cast<int>(static_cast<float>(myUnit.getAttackPower()) * env.getAtkMod());
    enemyUnit.receiveDamage(pAtk);

    std::string log = myUnit.getName() + " ataca (" + std::to_string(pAtk) + " dmg). ";

    if (!enemyUnit.isAlive()) {
        enemies.erase(enemies.begin());
        myUnit.gainExperience(75);
        if (enemies.empty()) {
            localCity.setOccupied(true);
            return log + "VICTORIE! Inamic eliminat, oras cucerit!";
        }
        return log + "Inamic rapus!";
    }

    int eAtk = static_cast<int>(static_cast<float>(enemyUnit.getAttackPower()) / env.getDefMod());
    myUnit.receiveDamage(eAtk);
    log += "Inamicul riposteaza (" + std::to_string(eAtk) + " dmg).";

    if (!myUnit.isAlive()) {
        p.removeDeadUnits();
        return log + " Unitatea ta a cazut in lupta!";
    }
    return log;
}
struct RecruitmentOption {
    UnitType type;
    std::string description;
    int cost;
    Ability specialAbility;
};

class RecruitmentCenter {
private:
    std::vector<RecruitmentOption> options;

public:
    RecruitmentCenter() {
        options.push_back({UnitType::INFANTERIE, "INFANTERIE: Defensiva superioara.", 100, Ability("Zid de Scuturi", 0.25f, 5)});
        options.push_back({UnitType::ARCASI, "ARCASI: Atac la distanta.", 130, Ability("Ploaie de Sageti", 0.35f, 20)});
        options.push_back({UnitType::CAVALERIE, "CAVALERIE: Soc si groaza.", 200, Ability("Sarja", 0.20f, 35)});
    }

    void draw(int gold) const {
        // UI adaptat pentru 1600x1200
        DrawRectangle(1000, 200, 550, 600, raylib::Color::LightGray().Alpha(0.9f));
        DrawRectangleLines(1000, 200, 550, 600, DARKGRAY);

        raylib::Text::Draw("CENTRUL DE RECRUTARE", 1050, 230, 35, DARKBLUE);
        raylib::Text::Draw("Aur: " + std::to_string(gold) + "g", 1050, 280, 25, DARKGREEN);

        int y = 350;
        int id = 1;
        for (const auto& opt : options) {
            UnitClassData data = UnitRegistry::GetData(opt.type);
            raylib::Text::Draw("[" + std::to_string(id) + "] " + data.className + " - " + std::to_string(opt.cost) + "g", 1050, y, 28, BLACK);
            raylib::Text::Draw(opt.description, 1050, y + 35, 18, DARKGRAY);
            y += 100;
            id++;
        }
    }

    bool process(Player& p, int idx, GameEngine::Logger& log) {
        if (idx < 0 || idx >= static_cast<int>(options.size())) return false;
        const auto& opt = options[static_cast<size_t>(idx)];
        if (p.spendGold(opt.cost)) {
            p.recruit(Unit(UnitRegistry::GetData(opt.type).className, opt.type, opt.specialAbility));
            log.add("Recrutat: " + UnitRegistry::GetData(opt.type).className);
            return true;
        }
        log.add("Fonduri insuficiente!");
        return false;
    }
};
class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    LoginSystem loginUI;
    Player player;
    RecruitmentCenter barracks;
    GameEngine::Logger logger;
    std::vector<Zone> worldMap;
    Quest mainQuest;
    int activeZoneIdx = 0;
    int day = 1;
    bool showRecruitment = false;

    void checkVictory() {
        bool allCaptured = true;
        for (const auto& z : worldMap) {
            if (!z.getCity().isOccupied()) { allCaptured = false; break; }
        }
        if (allCaptured) currentState = GameState::VICTORY;
    }

public:
    Simulation(int w, int h)
        : window(w, h, "EMPIRE RISING - SUPREME HEGEMONY"),
          currentState(GameState::LOGIN),
          player("Comandant", 600),
          mainQuest("Cuceritorul", "Ocupa toate orasele de pe harta", 1000)
    {
        SetTargetFPS(60);
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        worldMap.emplace_back("Valea Veridonia", Terrain("Campie", 1.2f, 1.0f, raylib::Color::Green()), City("Veridonia", 80));
        worldMap[0].spawnEnemy(Unit("Garda Neagra", UnitType::INFANTERIE, Ability("Lovitura", 0.1f, 10)));

        worldMap.emplace_back("Creasta de Fier", Terrain("Munte", 0.8f, 1.5f, raylib::Color::Gray()), City("Ironhold", 100));
        worldMap[1].spawnEnemy(Unit("Lordul Muntelui", UnitType::CAVALERIE, Ability("Sarja", 0.3f, 40)));
    }

    void handleInput() {
        if (IsKeyPressed(KEY_ESCAPE)) window.Close(); // Buton Escape

        if (currentState == GameState::LOGIN) {
            loginUI.update();
            if (loginUI.isFinished()) {
                player = Player(loginUI.getName(), 800);
                player.recruit(Unit("Garda Personala", UnitType::INFANTERIE, Ability("Protectie", 0.2f, 5)));
                currentState = GameState::SIMULATION;
            }
            return;
        }

        if (currentState == GameState::VICTORY) return;

        if (IsKeyPressed(KEY_N)) {
            day++;
            int taxes = player.collectTaxes(worldMap);
            logger.add("Ziua " + std::to_string(day) + ": Colectat " + std::to_string(taxes) + " aur.");
        }

        if (IsKeyPressed(KEY_F)) {
            logger.add(worldMap[static_cast<size_t>(activeZoneIdx)].battleRound(player));
            checkVictory();
        }

        if (IsKeyPressed(KEY_TAB)) activeZoneIdx = (activeZoneIdx + 1) % static_cast<int>(worldMap.size());
        if (IsKeyPressed(KEY_R)) showRecruitment = !showRecruitment;

        if (showRecruitment) {
            if (IsKeyPressed(KEY_ONE)) barracks.process(player, 0, logger);
            if (IsKeyPressed(KEY_TWO)) barracks.process(player, 1, logger);
            if (IsKeyPressed(KEY_THREE)) barracks.process(player, 2, logger);
        }
    }

    void render() {
        BeginDrawing();
        window.ClearBackground(RAYWHITE);

        if (currentState == GameState::LOGIN) {
            loginUI.draw();
        } else if (currentState == GameState::VICTORY) {
            DrawRectangle(0, 0, 1600, 1200, BLACK);
            raylib::Text::Draw("FELICITARI! AI CUCERIT INTREGUL IMPERIU!", 300, 500, 50, GameEngine::ColorPalette::Victory());
            raylib::Text::Draw("Comandantul " + player.getName() + " a adus pacea prin forta.", 450, 600, 30, WHITE);
            raylib::Text::Draw("Apasa ESCAPE pentru a iesi.", 600, 800, 20, LIGHTGRAY);
        } else {
            // UI SIMULATION 1600x1200
            DrawRectangle(0, 0, 1600, 150, GameEngine::ColorPalette::UIBack());
            raylib::Text::Draw("IMPIRUL LUI " + player.getName(), 50, 40, 40, WHITE);
            raylib::Text::Draw("TEZAUR: " + std::to_string(player.getGold()) + " AUR", 50, 90, 30, GameEngine::ColorPalette::Gold());
            raylib::Text::Draw("ZIUA " + std::to_string(day), 1400, 50, 40, LIGHTGRAY);

            // Quest vizibil sus in dreapta
            DrawRectangle(1100, 20, 450, 110, raylib::Color::Black().Alpha(0.5f));
            raylib::Text::Draw("MISIUNE: " + mainQuest.getTitle(), 1120, 40, 20, YELLOW);
            raylib::Text::Draw(mainQuest.getGoal(), 1120, 70, 18, WHITE);

            // Zona Activa
            auto& z = worldMap[static_cast<size_t>(activeZoneIdx)];
            DrawRectangle(50, 200, 900, 400, z.getTerrain().getColor().Alpha(0.4f));
            raylib::Text::Draw("ZONA: " + z.getName(), 80, 230, 35, BLACK);
            raylib::Text::Draw("STATUS ORAS: " + std::string(z.getCity().isOccupied() ? "CUCERIT" : "INAMIC"), 80, 280, 25, DARKGRAY);

            // Armata
            int y = 650;
            raylib::Text::Draw("TRUPELE TALE:", 50, 620, 25, DARKBLUE);
            for (const auto& u : player.getArmy()) {
                DrawRectangle(50, y, 400, 40, LIGHTGRAY);
                raylib::Text::Draw(u.getName() + " [Lvl " + std::to_string(u.getLvl()) + "]", 60, y + 10, 20, BLACK);
                y += 50; if (y > 1000) break;
            }

            // Log
            DrawRectangle(1000, 850, 550, 300, BLACK);
            int logY = 870;
            for (const auto& msg : logger.getMessages()) {
                raylib::Text::Draw(">> " + msg, 1020, logY, 16, GREEN);
                logY += 22;
            }

            if (showRecruitment) barracks.draw(player.getGold());

            // Footer
            DrawRectangle(0, 1150, 1600, 50, DARKGRAY);
            raylib::Text::Draw("[N] Tur Urmator | [F] Lupta | [R] Recrutare | [TAB] Schimba Zona | [ESC] Iesire", 400, 1165, 20, WHITE);
        }
        EndDrawing();
    }

    void run() {
        while (!window.ShouldClose()) {
            handleInput();
            render();
        }
    }
};
int main() {
    constexpr int screenWidth = 1600;
    constexpr int screenHeight = 1200;

    try {
        Simulation game(screenWidth, screenHeight);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}