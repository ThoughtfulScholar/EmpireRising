#ifndef ENUMS_H
#define ENUMS_H

#include <string>
#include <map>

// Stările generale ale jocului
enum class GameState {
    LOGIN,
    SIMULATION,
    VICTORIE,
    DEFEAT
};

// Tipurile de teren de pe hartă
enum class TerrainType {
    PLAIN,
    MOUNTAIN,
    FOREST,
    WATER,
    CITY_TILE
};

// Tipurile de unități recrutabile
enum class UnitType {
    INFANTERIE,
    ARCASI,
    CAVALERIE,
    GARDA,
    EROU
};



/*
// Structură de date pentru configurarea statică a trupelor
struct UnitStats {
    std::string name;
    int cost;
    int upkeep;
    int atk;
    int hp;
};

// Datele brute utilizate în ecranele de recrutare și fabrică
inline std::map<UnitType, UnitStats> GameData = {
    {UnitType::INFANTERIE, {"Infanterie", 150, 10, 25, 100}},
    {UnitType::ARCASI,     {"Arcasi", 200, 15, 35, 75}},
    {UnitType::CAVALERIE,  {"Cavalerie", 400, 30, 60, 150}},
    {UnitType::GARDA,      {"Garda", 180, 8, 20, 120}},
    {UnitType::EROU,       {"Erou", 1000, 100, 150, 500}}
};
*/

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
#endif // ENUMS_H