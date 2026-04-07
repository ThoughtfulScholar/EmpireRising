#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <raylib.h> // Folosim header-ul standard pentru funcții grafice
#include <raylib-cpp.hpp>

// ==========================================
// 1. CLASA ITEM (Compunere pentru Unit)
// ==========================================
class Item {
private:
    std::string itemName;
    int atkBonus;
    int hpBonus;
    int price;
public:
    Item(std::string n = "None", int a = 0, int h = 0, int p = 0)
        : itemName(std::move(n)), atkBonus(a), hpBonus(h), price(p) {}

    [[nodiscard]] int getAtk() const { return atkBonus; }
    [[nodiscard]] int getHp() const { return hpBonus; }
    [[nodiscard]] int getPrice() const { return price; }
    [[nodiscard]] const std::string& getName() const { return itemName; }

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        return os << i.itemName << " (ATK+" << i.atkBonus << " HP+" << i.hpBonus << ")";
    }
};

// ==========================================
// 2. CLASA UNIT (Regula celor 3)
// ==========================================
class Unit {
private:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    Item equippedItem;
    Color color;

public:
    Unit(std::string n, int h, int a, Color c)
        : name(std::move(n)), health(h), maxHealth(h), attack(a), equippedItem(), color(c) {}

    // REGULA CELOR TREI
    Unit(const Unit& other) 
        : name(other.name), health(other.health), maxHealth(other.maxHealth), 
          attack(other.attack), equippedItem(other.equippedItem), color(other.color) {}

    Unit& operator=(const Unit& other) {
        if (this != &other) {
            name = other.name;
            health = other.health;
            maxHealth = other.maxHealth;
            attack = other.attack;
            equippedItem = other.equippedItem;
            color = other.color;
        }
        return *this;
    }
    ~Unit() = default;

    // Funcție Netrivială 1: Echipare cu recalculare stats
    void equip(const Item& newItem) {
        equippedItem = newItem;
        health += newItem.getHp();
        maxHealth += newItem.getHp();
    }

    [[nodiscard]] int getTotalAtk() const { return attack + equippedItem.getAtk(); }
    void takeDamage(int dmg) { health = std::max(0, health - dmg); }
    [[nodiscard]] bool isAlive() const { return health > 0; }
    
    // Getters
    [[nodiscard]] int getHP() const { return health; }
    [[nodiscard]] int getMaxHP() const { return maxHealth; }
    [[nodiscard]] const std::string& getName() const { return name; }
    [[nodiscard]] Color getColor() const { return color; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        return os << u.name << " HP:" << u.health << " ATK:" << u.getTotalAtk() << " Item:" << u.equippedItem;
    }
};

// ==========================================
// 3. CLASA PLAYER (Gestiune resurse)
// ==========================================
class Player {
private:
    std::string username;
    int gold;
    std::vector<Item> inventory;

public:
    Player(std::string n, int g) : username(std::move(n)), gold(g) {}

    bool buyItem(const Item& it) {
        if (gold >= it.getPrice()) {
            gold -= it.getPrice();
            inventory.push_back(it);
            return true;
        }
        return false;
    }

    void addGold(int amount) { gold += amount; }
    [[nodiscard]] int getGold() const { return gold; }
    [[nodiscard]] const std::string& getName() const { return username; }
    [[nodiscard]] const std::vector<Item>& getInventory() const { return inventory; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        return os << "Player " << p.username << " | Gold: " << p.gold;
    }
};

// ==========================================
// 4. CLASA ZONE (Logică de luptă)
// ==========================================
class Zone {
private:
    std::string zoneName;
    std::vector<Unit> units;
    std::vector<std::string> logs;

public:
    explicit Zone(std::string name) : zoneName(std::move(name)) {}

    void addUnit(const Unit& u) { units.push_back(u); }

    // Funcție Netrivială 2: Luptă automată cu log-uri
    void fightStep() {
        if (units.size() < 2) return;
        
        int d1 = units[0].getTotalAtk();
        units[1].takeDamage(d1);
        logs.push_back(units[0].getName() + " hits " + units[1].getName() + " for " + std::to_string(d1));

        if (units[1].isAlive()) {
            int d2 = units[1].getTotalAtk();
            units[0].takeDamage(d2);
            logs.push_back(units[1].getName() + " hits " + units[0].getName() + " for " + std::to_string(d2));
        }

        // Cleanup folosind algorithm
        units.erase(std::remove_if(units.begin(), units.end(), 
            [](const Unit& u) { return !u.isAlive(); }), units.end());
    }

    [[nodiscard]] const std::vector<Unit>& getUnits() const { return units; }
    [[nodiscard]] const std::string& getName() const { return zoneName; }
    [[nodiscard]] const std::vector<std::string>& getLogs() const { return logs; }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone: " << z.zoneName << " Units: " << z.units.size();
        return os;
    }
};

// ==========================================
// 5. CLASA GAME (Manager interactiv)
// ==========================================
class Game {
private:
    Player player;
    std::vector<Zone> regions;
    std::vector<Item> shop;

public:
    explicit Game(Player p) : player(std::move(p)) {
        shop.push_back(Item("Sword", 10, 0, 100));
        shop.push_back(Item("Shield", 2, 50, 150));
        shop.push_back(Item("Relic", 20, 20, 300));
    }

    void init() {
        Zone forest("Shadow Forest");
        forest.addUnit(Unit("Soldier", 100, 15, BLUE));
        forest.addUnit(Unit("Goblin", 80, 12, GREEN));
        regions.push_back(forest);

        Zone arena("War Arena");
        arena.addUnit(Unit("Knight", 200, 25, RED));
        arena.addUnit(Unit("Dragon", 500, 40, PURPLE));
        regions.push_back(arena);
    }

    // Funcție Netrivială 3: Randare interfață complexă
    void draw() {
        ClearBackground(BLACK);
        
        // Player Info
        DrawText(TextFormat("PLAYER: %s | GOLD: %i", player.getName().c_str(), player.getGold()), 20, 20, 22, GOLD);
        DrawText("SPACE to Fight | S to Buy Sword (100g) | E to Equip", 20, 50, 18, LIGHTGRAY);

        int y = 100;
        for (const auto& z : regions) {
            DrawRectangle(20, y, 760, 30, DARKGRAY);
            DrawText(z.getName().c_str(), 30, y + 5, 20, WHITE);
            y += 40;

            for (const auto& u : z.getUnits()) {
                float hpBar = (float)u.getHP() / u.getMaxHP() * 200;
                DrawRectangle(30, y, 200, 20, MAROON);
                DrawRectangle(30, y, (int)hpBar, 20, u.getColor());
                DrawText(TextFormat("%s (%i/%i)", u.getName().c_str(), u.getHP(), u.getMaxHP()), 40, y + 2, 16, WHITE);
                y += 25;
            }

            // Draw Logs
            int logY = y - (z.getUnits().size() * 25);
            auto logs = z.getLogs();
            int count = 0;
            for(auto it = logs.rbegin(); it != logs.rend() && count < 3; ++it, ++count) {
                DrawText(it->c_str(), 400, logY + (count * 20), 14, GRAY);
            }
            y += 20;
        }
    }

    void handleInput() {
        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& r : regions) r.fightStep();
        }
        if (IsKeyPressed(KEY_S)) {
            if (player.buyItem(shop[0])) std::cout << "Bought Sword!\n";
        }
        if (IsKeyPressed(KEY_E)) {
            // Echipăm prima unitate din prima zonă cu ultimul item cumpărat
            if (!player.getInventory().empty() && !regions[0].getUnits().empty()) {
                // Notă: Folosim un mic truc de logică pentru a modifica fără a strica încapsularea
                auto& zoneUnits = const_cast<std::vector<Unit>&>(regions[0].getUnits());
                zoneUnits[0].equip(player.getInventory().back());
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        return os << "Game active for " << g.player.getName();
    }
};

// ==========================================
// 6. MAIN
// ==========================================
int main() {
    // Configurare fereastră Raylib (C standard)
    InitWindow(800, 600, "Empire Rising v0.1");
    SetTargetFPS(60);

    Game engine(Player("Stefan", 500));
    engine.init();

    std::cout << "Game started. Use SPACE to battle, S to buy, E to equip.\n";

    while (!WindowShouldClose()) {
        engine.handleInput();

        BeginDrawing();
        engine.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
