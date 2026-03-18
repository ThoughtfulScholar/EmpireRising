#include <iostream>
#include <string>
#include <vector>

// EmpireRising - Tema 1 (versiune finală, fără warnings)

class Unit {
private:
    std::string name;
    int health;
    int attack;

public:
    Unit(const std::string& name = "", int health = 0, int attack = 0)
        : name(name), health(health), attack(attack) {}

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool isAlive() const {
        return health > 0;
    }

    int getPower() const {
        return health + attack;
    }

    const std::string& getName() const { return name; }
    int getHealth() const { return health; }
    int getAttack() const { return attack; }

    friend std::ostream& operator<<(std::ostream& os, const Unit& u) {
        os << "Unit{name=" << u.name
           << ", health=" << u.health
           << ", attack=" << u.attack << "}";
        return os;
    }
};

class Player {
private:
    std::string name;
    int gold;
    std::vector<Unit> units;

public:
    Player(const std::string& name = "", int gold = 0)
        : name(name), gold(gold), units() {}

    // Regula celor trei (cerință)
    Player(const Player& other)
        : name(other.name), gold(other.gold), units(other.units) {}

    Player& operator=(const Player& other) {
        if (this != &other) {
            name = other.name;
            gold = other.gold;
            units = other.units;
        }
        return *this;
    }

    ~Player() {}

    void addUnit(const Unit& u) {
        units.push_back(u);
    }

    int getTotalPower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    bool spendGold(int amount) {
        if (amount > gold) return false;
        gold -= amount;
        return true;
    }

    const std::string& getName() const { return name; }
    int getGold() const { return gold; }

    const std::vector<Unit>& getUnits() const { return units; }

    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Player{name=" << p.name
           << ", gold=" << p.gold
           << ", units=[";
        for (size_t i = 0; i < p.units.size(); ++i) {
            os << p.units[i];
            if (i + 1 < p.units.size()) os << ", ";
        }
        os << "]}";
        return os;
    }
};

class Zone {
private:
    std::string name;
    std::vector<Unit> units;

public:
    Zone(const std::string& name = "")
        : name(name), units() {}

    ~Zone() = default;

    void addUnit(const Unit& u) {
        units.push_back(u);
    }

    int getZonePower() const {
        int total = 0;
        for (const auto& u : units) total += u.getPower();
        return total;
    }

    bool removeDeadUnits() {
        size_t before = units.size();
        std::vector<Unit> alive;
        alive.reserve(units.size());

        for (const auto& u : units)
            if (u.isAlive()) alive.push_back(u);

        units.swap(alive);
        return units.size() != before;
    }

    const std::string& getName() const { return name; }
    const std::vector<Unit>& getUnits() const { return units; }

    Unit getUnitAt(size_t index) const {
        return units.at(index);
    }

    void removeUnitAt(size_t index) {
        if (index < units.size())
            units.erase(units.begin() + index);
    }

    friend std::ostream& operator<<(std::ostream& os, const Zone& z) {
        os << "Zone{name=" << z.name
           << ", power=" << z.getZonePower()
           << ", units=[";
        for (size_t i = 0; i < z.units.size(); ++i) {
            os << z.units[i];
            if (i + 1 < z.units.size()) os << ", ";
        }
        os << "]}";
        return os;
    }
};

class Game {
private:
    std::vector<Player> players;
    std::vector<Zone> zones;

public:
    Game() = default;
    ~Game() = default;

    void addPlayer(const Player& p) {
        players.push_back(p);
    }

    void addZone(const Zone& z) {
        zones.push_back(z);
    }

    const std::vector<Player>& getPlayers() const { return players; }
    const std::vector<Zone>& getZones() const { return zones; }

    void moveUnit(Zone& from, size_t index, Zone& to) {
        if (index >= from.getUnits().size()) return;
        Unit u = from.getUnitAt(index);
        from.removeUnitAt(index);
        to.addUnit(u);
    }

    void battle(Zone& z) {
        auto& units = const_cast<std::vector<Unit>&>(z.getUnits());
        if (units.size() < 2) {
            std::cout << "Not enough units in zone " << z.getName() << "\n";
            return;
        }

        Unit& a = units[0];
        Unit& b = units[1];

        std::cout << "Battle in " << z.getName() << ": "
                  << a.getName() << " vs " << b.getName() << "\n";

        while (a.isAlive() && b.isAlive()) {
            b.takeDamage(a.getAttack());
            if (!b.isAlive()) break;
            a.takeDamage(b.getAttack());
        }

        z.removeDeadUnits();
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "EmpireRising Game{\n  Players:\n";
        for (const auto& p : g.players)
            os << "    " << p << "\n";
        os << "  Zones:\n";
        for (const auto& z : g.zones)
            os << "    " << z << "\n";
        os << "}";
        return os;
    }
};

int main() {
    std::cout << "=== Welcome to EmpireRising ===\n\n";

    Unit swordsman("Swordsman", 100, 20);
    Unit archer("Archer", 70, 25);
    Unit knight("Knight", 120, 30);

    Player p1("Player1", 100);
    Player p2("Player2", 80);

    p1.addUnit(swordsman);
    p1.addUnit(archer);
    p2.addUnit(knight);

    Zone forest("Forest");
    Zone hill("Hill");

    forest.addUnit(swordsman);
    forest.addUnit(knight);
    hill.addUnit(archer);

    Game game;
    game.addPlayer(p1);
    game.addPlayer(p2);
    game.addZone(forest);
    game.addZone(hill);

    std::cout << "Initial state:\n" << game << "\n\n";

    game.battle(game.getZones()[0]);

    if (!game.getZones()[1].getUnits().empty())
        game.moveUnit(game.getZones()[1], 0, game.getZones()[0]);

    p1.spendGold(30);

    std::cout << "\nAfter actions:\n" << game << "\n";

    return 0;
}