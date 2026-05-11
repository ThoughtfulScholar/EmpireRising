#ifndef UNIT_FACTORY_HPP
#define UNIT_FACTORY_HPP

#include "GlobalEnums.hpp"
#include "UnitTypes.hpp"
#include <map>
#include <string>
#include <memory>

struct UnitStats {
    std::string name;
    int hp;
    int atk;
    int cost;
    int upkeep;
};

// Tabelul de date global (extern pentru a fi definit în .cpp)
extern std::map<UnitType, UnitStats> GameData;

struct UnitFactory {
    static std::unique_ptr<Unit> CreateUnit(UnitType type, const std::string& customName = "");
};

#endif
