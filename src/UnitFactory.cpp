// #include "UnitFactory.h"
#include "../include/UnitFactory.h"

std::unique_ptr<Unit> UnitFactory::CreateUnit(UnitType type, const std::string& customName) {
    const auto& s = GameData.at(type);
    std::string fName = customName.empty() ? s.name : customName;

    switch (type) {
        case UnitType::INFANTERIE:
            return std::make_unique<Infantry>();

        case UnitType::ARCASI:
            return std::make_unique<Archer>();

        case UnitType::CAVALERIE:
            return std::make_unique<Cavalry>();

        case UnitType::GARDA:
            return std::make_unique<GarrisonGuard>(fName);

        case UnitType::EROU:
            return std::make_unique<Hero>(fName, s.hp, s.atk, s.upkeep);

        default:
            return nullptr;
    }
}