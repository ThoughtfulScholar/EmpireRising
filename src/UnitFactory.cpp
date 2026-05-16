// #include "UnitFactory.h"
#include "../include/UnitFactory.h"

std::unique_ptr<Unit> UnitFactory::CreateUnit(UnitType type, const std::string& customName) {
    const auto& stats = GameData[type];
    
    switch (type) {
        case UnitType::INFANTERIE:
            return std::make_unique<Infantry>(customName, stats.atk, stats.hp, stats.upkeep);
        case UnitType::ARCASI:
            return std::make_unique<Archer>(customName, stats.atk, stats.hp, stats.upkeep);
        case UnitType::CAVALERIE:
            return std::make_unique<Cavalry>(customName, stats.atk, stats.hp, stats.upkeep);
        case UnitType::GARDA:
            return std::make_unique<GarrisonGuard>(customName, stats.atk, stats.hp, stats.upkeep);
        case UnitType::EROU:
            return std::make_unique<Hero>(customName, stats.atk, stats.hp, stats.upkeep);
    }
    return nullptr;
}