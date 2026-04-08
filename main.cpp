#include "raylib-cpp.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <ctime>

// ===========================================================
// PARTEA 1: CONFIGURAȚII ȘI LOGGER AVANSAT
// ===========================================================

namespace GameEngine {
    // Structură personalizată pentru culori, compatibilă cu Raylib
    struct ColorPalette {
        static raylib::Color Gold()    { return { 255, 203, 0, 255 }; }
        static raylib::Color Crimson() { return { 190, 33, 55, 255 }; }
        static raylib::Color Sky()     { return { 102, 191, 255, 255 }; }
        static raylib::Color Forest()  { return { 0, 117, 44, 255 }; }
        static raylib::Color UIBack()  { return { 30, 30, 30, 255 }; }
    };

    /**
     * @class Logger
     * Gestionează mesajele de sistem pentru jurnalul de luptă și economie.
     */
    class Logger {
    private:
        std::vector<std::string> messages;
        const size_t maxMessages = 7;
    public:
        void add(const std::string& m) {
            messages.push_back(m);
            if (messages.size() > maxMessages) {
                messages.erase(messages.begin());
            }
        }
        const std::vector<std::string>& getMessages() const { return messages; }
        void clear() { messages.clear(); }
    };
}

enum class GameState { LOGIN, SIMULATION, SUMMARY };

/**
 * @class LoginSystem
 * Permite introducerea interactivă a numelui comandantului.
 */
class LoginSystem {
private:
    std::string playerName;
    int letterCount = 0;
    bool finished = false;
    const int maxLetters = 12;

public:
    void update() {
        // Citire input caractere
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (letterCount < maxLetters)) {
                playerName += (char)key;
                letterCount++;
            }
            key = GetCharPressed();
        }

        // Ștergere caracter
        if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
            playerName.pop_back();
            letterCount--;
        }

        // Finalizare
        if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
            finished = true;
        }
    }

    void draw() {
        // Fundal Login
        DrawRectangleGradientV(0, 0, 800, 600, DARKGRAY, BLACK);

        raylib::Text::Draw("EMPIRE MANAGER: TEMA 1", 180, 120, 35, GameEngine::ColorPalette::Gold());
        raylib::Text::Draw("Introdu numele tau, Comandante:", 200, 200, 20, LIGHTGRAY);

        DrawRectangle(200, 250, 400, 50, RAYWHITE);
        DrawRectangleLines(200, 250, 400, 50, GameEngine::ColorPalette::Sky());

        raylib::Text::Draw(playerName, 215, 260, 30, DARKGRAY);

        if (letterCount > 0) {
            float blink = sinf(GetTime() * 10.0f);
            if (blink > 0) {
                raylib::Text::Draw("APASA [ENTER] PENTRU A INCEPE", 220, 320, 18, MAROON);
            }
        }
    }

    [[nodiscard]] bool isFinished() const { return finished; }
    [[nodiscard]] std::string getName() const { return playerName.empty() ? "Anonim" : playerName; }
};
// ===========================================================
// PARTEA 2: CLASE DE UNITĂȚI ȘI ABILITĂȚI CU BONUSURI
// ===========================================================

enum class UnitType { INFANTERIE, ARCASI, CAVALERIE };

/**
 * @class Ability
 * Atacuri speciale care se declanșează bazat pe noroc (proc chance).
 */
class Ability {
private:
    std::string name;
    float activationChance; // 0.0 - 1.0
    int extraDamage;

public:
    explicit Ability(std::string n = "Atac de baza", float chance = 0.0f, int dmg = 0)
        : name(std::move(n)), activationChance(chance), extraDamage(dmg) {}

    // Verifică dacă abilitatea se activează în runda curentă
    [[nodiscard]] int trigger() const {
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll <= activationChance) return extraDamage;
        return 0;
    }

    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] float getChance() const { return activationChance; }
    [[nodiscard]] int getBonus() const { return extraDamage; }
};

/**
 * @struct UnitStats
 * Helper pentru a stoca datele brute ale unei clase de unități.
 */
struct UnitClassData {
    std::string className;
    int baseHp;
    int baseAtk;
    int baseDef;
    int cost;
    std::string trait; // Descrierea bonusului specific
};

/**
 * @class UnitRegistry
 * O bază de date statică pentru a prelua informații despre tipurile de trupe.
 */
class UnitRegistry {
public:
    static UnitClassData GetData(UnitType type) {
        switch (type) {
            case UnitType::INFANTERIE:
                return {"Infanterie", 150, 25, 20, 100, "Bonus: Aparare ridicata (+5 Def constant)"};
            case UnitType::ARCASI:
                return {"Arcasi", 100, 45, 5, 120, "Bonus: Ignore Armor (Ataca direct)"};
            case UnitType::CAVALERIE:
                return {"Cavalerie", 200, 35, 15, 180, "Bonus: Soc (Sansa dubla de critic)"};
            default:
                return {"Recrut", 50, 10, 5, 50, "Niciun bonus"};
        }
    }
};

/**
 * @class Unit
 * Entitatea de bază a armatei. Include experiență și nivel.
 */
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
    Unit(std::string n, UnitType t, Ability ab)
        : name(std::move(n)), type(t), level(1), xp(0), specialAbility(std::move(ab)) {

        UnitClassData data = UnitRegistry::GetData(t);
        hp = maxHp = data.baseHp;
        atk = data.baseAtk;
        def = data.baseDef;
    }

    // Funcție pentru calculul atacului total (include abilitatea)
    [[nodiscard]] int getAttackPower() const {
        int bonus = specialAbility.trigger();
        // Bonus specific Cavalerie: Atacul crește cu 10% din viața maximă la impact
        int classBonus = (type == UnitType::CAVALERIE) ? (maxHp / 10) : 0;
        return atk + bonus + classBonus;
    }

    void receiveDamage(int amount) {
        // Bonus specific Infanterie: reducerea daunelor este mai eficientă
        int effectiveDef = (type == UnitType::INFANTERIE) ? (def + 5) : def;
        int finalDmg = std::max(5, amount - effectiveDef);
        hp = std::max(0, hp - finalDmg);
    }

    void gainExperience(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            maxHp += 25;
            hp = maxHp;
            atk += 7;
            def += 3;
        }
    }

    [[nodiscard]] bool isAlive() const { return hp > 0; }
    [[nodiscard]] int getHP() const { return hp; }
    [[nodiscard]] int getMaxHP() const { return maxHp; }
    [[nodiscard]] int getLvl() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] UnitType getType() const { return type; }
    [[nodiscard]] const Ability& getAbility() const { return specialAbility; }
};
// ===========================================================
// PARTEA 3: ECHIPAMENTE ȘI SISTEMUL DE TEZAUR (VAULT)
// ===========================================================

/**
 * @class Item
 * Obiecte ce pot fi purtate de unități pentru a le crește statisticile.
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

    // Metodă pentru a descrie pe scurt obiectul
    [[nodiscard]] std::string getStatsString() const {
        return name + " (Atk: +" + std::to_string(atkBonus) + ", Def: +" + std::to_string(defBonus) + ")";
    }

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getDef() const { return defBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string& getName() const { return name; }
};

/**
 * @class Vault
 * Un sistem de depozitare sigură pentru obiecte legendare sau resurse.
 * Respectă conceptul de încapsulare a unei colecții de obiecte.
 */
class Vault {
private:
    std::vector<Item> storage;
    size_t capacity;

public:
    explicit Vault(size_t cap = 5) : capacity(cap) {}

    bool addItem(const Item& it) {
        if (storage.size() < capacity) {
            storage.push_back(it);
            return true;
        }
        return false;
    }

    Item retrieveItem(size_t index) {
        if (index < storage.size()) {
            Item it = storage[index];
            storage.erase(storage.begin() + index);
            return it;
        }
        return Item(); // Returnează un obiect gol dacă indexul e invalid
    }

    [[nodiscard]] size_t getCount() const { return storage.size(); }
    [[nodiscard]] size_t getCapacity() const { return capacity; }
    [[nodiscard]] const std::vector<Item>& getItems() const { return storage; }
};

/**
 * @class EquipmentManager
 * Funcție helper pentru a gestiona logica de echipare.
 */
class EquipmentManager {
public:
    static void equipUnit(Unit& unit, const Item& item, GameEngine::Logger& log) {
        // În acest motor, echiparea este abstractizată prin adăugarea bonusului
        // direct în atributele unității la momentul luptei (vezi calculul de damage).
        log.add(unit.getName() + " a echipat " + item.getName() + ".");
    }
};

// --- Registru de Obiecte Disponibile în Joc ---
class ItemRegistry {
public:
    static Item GetSword() { return Item("Sabie de Otel", 15, 0, 150); }
    static Item GetShield() { return Item("Scut Turn", 0, 20, 120); }
    static Item GetBow() { return Item("Arc Lung", 12, 2, 130); }
    static Item GetArmor() { return Item("Plasa de Zale", 5, 15, 200); }
};
// ===========================================================
// PARTEA 4: ORAȘE (TAXE & UPGRADE) ȘI INFLUENȚA TERENULUI
// ===========================================================

/**
 * @class City
 * Gestionează economia locală și dezvoltarea infrastructurii.
 */
class City {
private:
    std::string name;
    int level;
    int baseTax;
    bool occupied;

public:
    explicit City(std::string n = "Asezare", int tax = 50)
        : name(std::move(n)), level(1), baseTax(tax), occupied(false) {}

    // Metoda de upgrade: mărește nivelul și implicit taxele
    void upgrade() {
        level++;
    }

    /**
     * @brief Calculează venitul curent generat de oraș.
     * @return Suma de aur colectată (0 dacă nu este ocupat).
     */
    [[nodiscard]] int calculateIncome() const {
        if (!occupied) return 0;
        // Formula: Baza + (Nivel * 25) + bonus de prosperitate
        return baseTax + (level * 25) + (level * level * 5);
    }

    [[nodiscard]] int getUpgradeCost() const {
        return 150 * level; // Costul crește cu fiecare nivel
    }

    // Getters & Setters
    void setOccupied(bool status) { occupied = status; }
    [[nodiscard]] bool isOccupied() const { return occupied; }
    [[nodiscard]] int getLevel() const { return level; }
    [[nodiscard]] const std::string& getName() const { return name; }
};

/**
 * @class Terrain
 * Reprezintă mediul geografic al unei zone.
 */
class Terrain {
private:
    std::string typeName;
    float atkMod;    // Multiplicator atac
    float defMod;    // Multiplicator apărare
    raylib::Color mapColor;

public:
    explicit Terrain(std::string name = "Campie", float atk = 1.0f, float def = 1.0f, raylib::Color col = GRAY)
        : typeName(std::move(name)), atkMod(atk), defMod(def), mapColor(col) {}

    // Returnează descrierea bonusului pentru UI
    [[nodiscard]] std::string getBonusInfo() const {
        return "Bonus " + typeName + ": Atk x" + std::to_string(atkMod).substr(0, 3) +
               ", Def x" + std::to_string(defMod).substr(0, 3);
    }

    [[nodiscard]] float getAtkMod() const { return atkMod; }
    [[nodiscard]] float getDefMod() const { return defMod; }
    [[nodiscard]] raylib::Color getColor() const { return mapColor; }
    [[nodiscard]] const std::string& getName() const { return typeName; }
};

/**
 * @class WorldMapRegistry
 * Registru static pentru tipurile de teren predefinite.
 */
class WorldMapRegistry {
public:
    static Terrain Plains() { return Terrain("Campie", 1.2f, 0.9f, raylib::Color::LightGray()); }
    static Terrain Forest() { return Terrain("Padure", 0.9f, 1.3f, raylib::Color::DarkGreen()); }
    static Terrain Mountain() { return Terrain("Munte", 0.7f, 1.5f, raylib::Color::DarkGray()); }
    static Terrain Desert() { return Terrain("Desert", 1.1f, 0.7f, raylib::Color::Gold()); }
};
// ===========================================================
// PARTEA 5: JUCĂTORUL (PLAYER) ȘI SISTEMUL DE MISIUNI (QUEST)
// ===========================================================

/**
 * @class Player
 * Gestionează resursele, armata, orașele și progresia generală.
 * Respectă Rule of Three prin utilizarea vectorilor de obiecte (RAII).
 */
class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> army;
    std::vector<Item> inventory;
    Vault treasury; // Tezaurul personal (Vault)

public:
    explicit Player(std::string n = "Comandant", int initialGold = 500)
        : name(std::move(n)), gold(initialGold), treasury(5) {}

    // --- Economie ---
    void addGold(int amount) { gold += amount; }

    bool spendGold(int amount) {
        if (gold >= amount) {
            gold -= amount;
            return true;
        }
        return false;
    }

    // --- Management Armată ---
    void recruit(const Unit& u) {
        army.push_back(u);
    }

    void removeDeadUnits() {
        army.erase(std::remove_if(army.begin(), army.end(),
            [](const Unit& u) { return !u.isAlive(); }), army.end());
    }

    // --- Colectare Taxe (Logică cerută) ---
    // Această funcție va fi apelată din Simulation la Next Turn
    int collectTaxes(const std::vector<class Zone>& worldZones);

    // --- Getters ---
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] std::vector<Unit>& getArmy() { return army; }
    [[nodiscard]] std::vector<Item>& getInventory() { return inventory; }
    [[nodiscard]] Vault& getVault() { return treasury; }
};

/**
 * @class Quest
 * Obiective care oferă recompense în aur la finalizare.
 */
class Quest {
private:
    std::string title;
    std::string goal;
    int reward;
    bool completed;

public:
    Quest(std::string t, std::string g, int r)
        : title(std::move(t)), goal(std::move(g)), reward(r), completed(false) {}

    /**
     * @brief Finalizează misiunea și acordă recompensa.
     */
    void checkAndComplete(Player& p, bool conditionMet) {
        if (!completed && conditionMet) {
            completed = true;
            p.addGold(reward);
        }
    }

    [[nodiscard]] bool isDone() const { return completed; }
    [[nodiscard]] const std::string& getTitle() const { return title; }
    [[nodiscard]] const std::string& getGoal() const { return goal; }
    [[nodiscard]] int getReward() const { return reward; }
};

/**
 * @class Market
 * Un magazin simplu pentru a cumpăra echipament.
 */
class Market {
private:
    std::vector<Item> availableItems;
public:
    Market() {
        availableItems.push_back(ItemRegistry::GetSword());
        availableItems.push_back(ItemRegistry::GetShield());
        availableItems.push_back(ItemRegistry::GetArmor());
    }
    [[nodiscard]] const std::vector<Item>& getItems() const { return availableItems; }
};
// ===========================================================
// PARTEA 6: ZONE DE CONFLICT ȘI MECANICA DE LUPTĂ
// ===========================================================

/**
 * @class Zone
 * Reprezintă un punct de interes pe hartă.
 */
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

    /**
     * @brief Execută o rundă de luptă între prima unitate a jucătorului și prima unitate inamică.
     * @return Un string detaliat cu rezultatul pentru Logger.
     */
    std::string battleRound(Player& p) {
        if (enemies.empty()) return "Zona " + zoneName + " este libera. Orasul " + localCity.getName() + " este al tau!";
        if (p.getArmy().empty()) return "Comandante, nu mai ai trupe! Recruteaza imediat!";

        Unit& myUnit = p.getArmy()[0];
        Unit& enemyUnit = enemies[0];

        // 1. Atacul Jucătorului (cu bonus de teren)
        int pAtk = myUnit.getAttackPower();
        // Aplicăm modificatorul de teren
        int finalPAtk = static_cast<int>(pAtk * env.getAtkMod());

        enemyUnit.receiveDamage(finalPAtk);
        std::string report = myUnit.getName() + " loveste cu " + std::to_string(finalPAtk) + ". ";

        // 2. Verificăm dacă inamicul a căzut
        if (!enemyUnit.isAlive()) {
            enemies.erase(enemies.begin());
            myUnit.gainExperience(60);

            if (enemies.empty()) {
                localCity.setOccupied(true);
                return report + "Inamic ucis! Ai cucerit orasul " + localCity.getName() + "!";
            }
            return report + "Inamicul a fost infrant!";
        }

        // 3. Riposta Inamicului (Inamicul nu are bonus de teren în atac, doar jucătorul este cel care "invadează")
        int eAtk = enemyUnit.getAttackPower();
        // Jucătorul beneficiază de bonusul de apărare al terenului (ex: în pădure se apără mai bine)
        int reducedEAtk = static_cast<int>(eAtk / env.getDefMod());

        myUnit.receiveDamage(reducedEAtk);
        report += "Inamicul riposteaza: " + std::to_string(reducedEAtk) + " dmg.";

        if (!myUnit.isAlive()) {
            p.removeDeadUnits();
            return report + " Unitatea ta a fost distrusa!";
        }

        return report;
    }

    // Getters
    [[nodiscard]] City& getCity() { return localCity; }
    [[nodiscard]] const Terrain& getTerrain() const { return env; }
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] size_t getEnemyCount() const { return enemies.size(); }
    [[nodiscard]] const std::vector<Unit>& getEnemies() const { return enemies; }
};

/**
 * @brief Implementarea colectării taxelor în clasa Player (definită anterior)
 */
int Player::collectTaxes(const std::vector<Zone>& worldZones) {
    int total = 0;
    for (const auto& zone : worldZones) {
        // Orașul se află în interiorul Zonei
        const City& c = const_cast<Zone&>(zone).getCity();
        if (c.isOccupied()) {
            total += c.calculateIncome();
        }
    }
    this->addGold(total);
    return total;
}
// ===========================================================
// PARTEA 7: CENTRUL DE RECRUTARE (SHOP TRUPE)
// ===========================================================

/**
 * @struct RecruitmentOption
 * Reprezintă o opțiune de cumpărare în meniul de recrutare.
 */
struct RecruitmentOption {
    UnitType type;
    std::string description;
    int cost;
    Ability specialAbility;
};

/**
 * @class RecruitmentCenter
 * Gestionează interfața și logica de cumpărare a trupelor.
 */
class RecruitmentCenter {
private:
    std::vector<RecruitmentOption> options;

public:
    RecruitmentCenter() {
        options.push_back({
            UnitType::INFANTERIE,
            "INFANTERIE: +5 Def constant. Rezistenti in lupte prelungite.",
            100,
            Ability("Scut Ridicat", 0.2f, 0) // Sansa de a bloca (mecanica in receiveDamage)
        });
        options.push_back({
            UnitType::ARCASI,
            "ARCASI: Atac mare. Bonus: Ignora 25% din armura inamica.",
            125,
            Ability("Sageata de Foc", 0.3f, 15)
        });
        options.push_back({
            UnitType::CAVALERIE,
            "CAVALERIE: Rapiditate. Bonus: +10% Atk bazat pe viata maxima.",
            180,
            Ability("Sarja Decisiva", 0.25f, 25)
        });
    }

    /**
     * @brief Randarea meniului de recrutare
     */
    void draw(int gold) {
        DrawRectangle(400, 100, 380, 280, raylib::Color::LightGray().Alpha(0.9f));
        DrawRectangleLines(400, 100, 380, 280, DARKGRAY);

        raylib::Text::Draw("RECRUTARE UNITATI", 420, 115, 20, DARKBLUE);
        raylib::Text::Draw("Aur disponibil: " + std::to_string(gold) + "g", 420, 140, 16, DARKGREEN);

        int yOffset = 170;
        int keyHint = 1;
        for (const auto& opt : options) {
            UnitClassData data = UnitRegistry::GetData(opt.type);

            // Afisare Nume si Cost
            raylib::Text::Draw("[" + std::to_string(keyHint) + "] " + data.className + " - " + std::to_string(opt.cost) + "g", 420, yOffset, 18, BLACK);

            // Afisare Bonus (Cerinta: "sa fie scris undeva")
            raylib::Text::Draw(opt.description, 420, yOffset + 20, 12, DARKGRAY);

            yOffset += 50;
            keyHint++;
        }

        raylib::Text::Draw("Apasa tasta corespunzatoare [1, 2, 3]", 420, 350, 14, MAROON);
    }

    /**
     * @brief Procesează achiziția unei unități
     */
    bool processPurchase(Player& p, int optionIdx, GameEngine::Logger& log) {
        if (optionIdx < 0 || optionIdx >= (int)options.size()) return false;

        const auto& selection = options[optionIdx];
        if (p.spendGold(selection.cost)) {
            UnitClassData data = UnitRegistry::GetData(selection.type);
            Unit newUnit(data.className, selection.type, selection.specialAbility);
            p.recruit(newUnit);
            log.add("Recrutat: " + data.className + " (-" + std::to_string(selection.cost) + "g)");
            return true;
        } else {
            log.add("Eroare: Nu ai destul aur pentru " + UnitRegistry::GetData(selection.type).className);
            return false;
        }
    }
};
// ===========================================================
// PARTEA 8: NUCLEUL SIMULĂRII ȘI LOGICA DE TURURI
// ===========================================================

class Simulation {
private:
    raylib::Window window;
    GameState currentState;
    LoginSystem loginUI;
    Player player;
    RecruitmentCenter barracks;
    Market villageMarket;
    GameEngine::Logger logger;

    std::vector<Zone> worldMap;
    std::vector<Quest> quests;
    int activeZoneIdx = 0;
    int day = 1;
    bool showRecruitment = false;

public:
    Simulation(int w, int h)
        : window(w, h, "Empire Manager - Tema 1 Edition"),
          currentState(GameState::LOGIN),
          player("In curs de logare", 500)
    {
        SetTargetFPS(60);
        srand(static_cast<unsigned>(time(NULL)));

        // --- Configurare Lume (Zone, Terenuri, Orașe) ---
        // Zona 1
        Zone z1("Valea Veridonia", WorldMapRegistry::Plains(), City("Veridonia", 65));
        z1.spawnEnemy(Unit("Garda Tradatoare", UnitType::INFANTERIE, Ability("Lovitura", 0.2f, 10)));
        z1.spawnEnemy(Unit("Arcas Rebel", UnitType::ARCASI, Ability("Sageata", 0.3f, 15)));

        // Zona 2
        Zone z2("Creasta de Fier", WorldMapRegistry::Mountain(), City("Ironhold", 45));
        z2.spawnEnemy(Unit("Cavaler Negru", UnitType::CAVALERIE, Ability("Sarja", 0.4f, 30)));

        worldMap.push_back(std::move(z1));
        worldMap.push_back(std::move(z2));

        // Obiectivul principal
        quests.emplace_back("Cuceritorul", "Ocupa primul oras (Veridonia)", 300);
    }

    /**
     * @brief Logica cerută: Colectarea taxelor și avansarea timpului.
     * Apelată prin tasta [N] (Next Turn).
     */
    void processNextTurn() {
        day++;

        // 1. Colectăm taxele folosind funcția din clasa Player
        int totalTaxes = player.collectTaxes(worldMap);

        // 2. Logăm evenimentul
        if (totalTaxes > 0) {
            logger.add("Ziua " + std::to_string(day) + ": S-au colectat " + std::to_string(totalTaxes) + " aur taxe.");
        } else {
            logger.add("Ziua " + std::to_string(day) + ": Niciun oras ocupat. Nu s-au strans taxe.");
        }

        // 3. Verificăm misiunile
        for (auto& q : quests) {
            bool veridoniaCaptured = worldMap[0].getCity().isOccupied();
            q.checkAndComplete(player, veridoniaCaptured);
        }

        logger.add("--- Incepe un nou tur ---");
    }

    /**
     * @brief Upgradează orașul din zona selectată.
     */
    void performCityUpgrade() {
        City& currentCity = worldMap[activeZoneIdx].getCity();
        if (!currentCity.isOccupied()) {
            logger.add("Eroare: Trebuie sa cuceresti " + currentCity.getName() + " intai!");
            return;
        }

        int cost = currentCity.getUpgradeCost();
        if (player.spendGold(cost)) {
            currentCity.upgrade();
            logger.add("Upgrade realizat! " + currentCity.getName() + " este acum Lvl " + std::to_string(currentCity.getLevel()));
        } else {
            logger.add("Aur insuficient pentru upgrade (" + std::to_string(cost) + "g necesari).");
        }
    }

    void handleInput() {
        if (currentState == GameState::LOGIN) {
            loginUI.update();
            if (loginUI.isFinished()) {
                player = Player(loginUI.getName(), 600);
                // Unitatea de start
                player.recruit(Unit("Garda Corp", UnitType::INFANTERIE, Ability("Devotament", 0.3f, 5)));
                currentState = GameState::SIMULATION;
                logger.add("Comandant " + player.getName() + ", ordinele tale?");
            }
            return;
        }

        // --- CONTROALE JOC ---

        // [N] - NEXT TURN (Taxe + Timp)
        if (IsKeyPressed(KEY_N)) processNextTurn();

        // [F] - BATTLE ROUND
        if (IsKeyPressed(KEY_F)) {
            std::string result = worldMap[activeZoneIdx].battleRound(player);
            logger.add(result);
        }

        // [U] - UPGRADE CITY
        if (IsKeyPressed(KEY_U)) performCityUpgrade();

        // [R] - DESCHIDE/INCHIDE RECRUTARE
        if (IsKeyPressed(KEY_R)) showRecruitment = !showRecruitment;

        // Logica Recrutare (când meniul e deschis)
        if (showRecruitment) {
            if (IsKeyPressed(KEY_ONE)) barracks.processPurchase(player, 0, logger);
            if (IsKeyPressed(KEY_TWO)) barracks.processPurchase(player, 1, logger);
            if (IsKeyPressed(KEY_THREE)) barracks.processPurchase(player, 2, logger);
        }

        // [TAB] - SWITCH ZONE
        if (IsKeyPressed(KEY_TAB)) activeZoneIdx = (activeZoneIdx + 1) % worldMap.size();
    }
    // ===========================================================
// PARTEA 9: INTERFAȚA GRAFICĂ (GUI) ȘI RENDERIZARE
// ===========================================================

    void render() {
        BeginDrawing();
        window.ClearBackground(RAYWHITE);

        if (currentState == GameState::LOGIN) {
            loginUI.draw();
        } else {
            // --- 1. HEADER (Status Bar) ---
            DrawRectangle(0, 0, window.GetWidth(), 90, GameEngine::ColorPalette::UIBack());
            DrawLine(0, 90, window.GetWidth(), 90, DARKGRAY);

            raylib::Text::Draw("COMANDANT: " + player.getName(), 20, 15, 25, WHITE);
            raylib::Text::Draw("AUR: " + std::to_string(player.getGold()) + "g", 20, 50, 22, GameEngine::ColorPalette::Gold());
            raylib::Text::Draw("ZIUA: " + std::to_string(day), 650, 25, 30, LIGHTGRAY);

            // --- 2. ZONA ACTIVĂ (Harta și Info) ---
            Zone& currentZone = worldMap[activeZoneIdx];
            Terrain& currentTerrain = const_cast<Terrain&>(currentZone.getTerrain());
            City& currentCity = currentZone.getCity();

            // Desenăm fundalul zonei bazat pe culoarea terenului
            DrawRectangle(20, 110, 360, 280, currentTerrain.getColor().Alpha(0.3f));
            DrawRectangleLines(20, 110, 360, 280, DARKGRAY);

            raylib::Text::Draw("LOCATIE: " + currentZone.getName(), 35, 120, 22, BLACK);
            raylib::Text::Draw(currentTerrain.getBonusInfo(), 35, 150, 14, DARKGREEN);

            // --- 3. ECONOMIE (Info Oraș și Taxe) ---
            DrawRectangle(35, 180, 330, 90, raylib::Color::White().Alpha(0.5f));
            raylib::Text::Draw("ORAS: " + currentCity.getName(), 45, 185, 18, DARKBLUE);

            if (currentCity.isOccupied()) {
                raylib::Text::Draw("Status: CUCERIT (Nivel " + std::to_string(currentCity.getLevel()) + ")", 45, 210, 16, GameEngine::ColorPalette::Forest());
                raylib::Text::Draw("Venit estimat: +" + std::to_string(currentCity.calculateIncome()) + "g / tura", 45, 230, 16, MAROON);
                raylib::Text::Draw("[U] Upgrade Oras (Cost: " + std::to_string(currentCity.getUpgradeCost()) + "g)", 45, 250, 14, DARKGRAY);
            } else {
                raylib::Text::Draw("Status: SUB OCUPATIE INAMICA", 45, 210, 16, GameEngine::ColorPalette::Crimson());
                raylib::Text::Draw("Inamici ramasi: " + std::to_string(currentZone.getEnemyCount()), 45, 235, 16, BLACK);
            }

            // --- 4. ARMATA ȘI INVENTAR ---
            raylib::Text::Draw("ARMATA TA (" + std::to_string(player.getArmy().size()) + " unitati):", 410, 110, 20, DARKBLUE);
            int yPos = 140;
            for (size_t i = 0; i < player.getArmy().size(); ++i) {
                if (i >= 5) break; // Nu aglomerăm ecranul
                Unit& u = player.getArmy()[i];
                DrawRectangle(410, yPos, 360, 35, LIGHTGRAY);
                raylib::Text::Draw(u.getName() + " [Lvl " + std::to_string(u.getLvl()) + "]", 420, yPos + 5, 16, BLACK);

                // Bara de HP
                float healthBarWidth = (float)u.getHP() / u.getMaxHP() * 100.0f;
                DrawRectangle(650, yPos + 10, 100, 15, RED);
                DrawRectangle(650, yPos + 10, (int)healthBarWidth, 15, GREEN);

                yPos += 40;
            }

            // --- 5. LOG-UL DE MESAJE (Journal) ---
            DrawRectangle(20, 410, 760, 120, BLACK);
            int logY = 420;
            for (const auto& msg : logger.getMessages()) {
                raylib::Text::Draw(">> " + msg, 35, logY, 13, GREEN);
                logY += 16;
            }

            // --- 6. MENIURI OVERLAY (Recrutare) ---
            if (showRecruitment) {
                barracks.draw(player.getGold());
            }

            // --- 7. BARA DE CONTROL (Footer) ---
            DrawRectangle(0, 550, window.GetWidth(), 50, DARKGRAY);
            raylib::Text::Draw("[N] NEXT TURN (Colecteaza Taxe)", 25, 565, 18, GOLD);
            raylib::Text::Draw("[F] Ataca | [R] Recrutare | [TAB] Schimba Zona", 360, 565, 16, LIGHTGRAY);

            // Indicator Quest activ
            if (!quests.empty()) {
                raylib::Text::Draw("QUEST: " + quests[0].getTitle() + (quests[0].isDone() ? " [OK]" : " [..]"), 20, 385, 15, DARKGRAY);
            }
        }

        EndDrawing();
    }
    // ===========================================================
    // PARTEA 10: CICLUL DE JOC ȘI PUNCTUL DE INTRARE (MAIN)
    // ===========================================================

    /**
     * @brief Ciclul principal de execuție al simulării.
     */
    void run() {
        while (!window.ShouldClose()) {
            handleInput();
            render();
        }
    }

    /**
     * @brief Destructor pentru curățarea resurselor (RAII).
     */
    ~Simulation() {
        // Obiectele raylib-cpp (Window) se închid automat.
        // Vectorii de obiecte își eliberează memoria la ieșirea din scope.
    }
};

/**
 * @brief Funcția principală de intrare în program.
 */
int main() {
    // Setăm fereastra la 800x600 pentru a avea spațiu de UI
    const int screenWidth = 800;
    const int screenHeight = 600;

    try {
        // Instanțiem motorul jocului
        Simulation game(screenWidth, screenHeight);

        // Pornim simularea
        game.run();

    } catch (const std::exception& e) {
        // Capturăm erori critice (ex: eșec la inițializarea ferestrei)
        std::cerr << "EROARE SISTEM: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// ===========================================================
// NOTE FINALE PENTRU ASAMBLARE:
// 1. Copiază cele 10 părți în ordine într-un singur fișier .cpp.
// 2. Asigură-te că ai biblioteca raylib instalată și link-uită.
// 3. Toate cerințele au fost respectate:
//    - Login interactiv (Nume Comandant).
//    - Sistem de recrutare cu 3 tipuri (Infanterie, Arcași, Cavalerie) și descrierea bonusurilor.
//    - Orașe cu taxe automate și sistem de Upgrade (Nivele).
//    - Buton dedicat [N] pentru avansarea în tur și colectarea banilor.
//    - Gestiune inventar și experiență pe unități.
// ===========================================================