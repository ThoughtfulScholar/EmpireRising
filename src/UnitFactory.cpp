#include "../include/UnitFactory.hpp"

// Inițializarea bazei de date a unităților
std::map<UnitType, UnitStats> GameData = {
    {UnitType::INFANTERIE, {"Infanterie", 300, 45, 150, 25}},
    {UnitType::ARCASI,     {"Arcas",      180, 70, 180, 30}},
    {UnitType::CAVALERIE,  {"Cavalerie",  350, 60, 300, 50}},
    {UnitType::GARDA,      {"Garda",      250, 40, 100, 15}},
    {UnitType::EROU,       {"Erou",       600, 90, 1000, 100}}
};

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
