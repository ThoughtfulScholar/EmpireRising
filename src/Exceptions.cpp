// #include "Exceptions.h"
#include "../include/Exceptions.h"

EmpireException::EmpireException(const std::string& msg) 
    : std::runtime_error(msg) {}

GoldException::GoldException(const std::string& msg) 
    : EmpireException("ERR_GOLD: " + msg) {}

PopulationLimitException::PopulationLimitException(const std::string& msg) 
    : EmpireException("ERR_POPULATION: " + msg) {}

MapException::MapException(const std::string& msg) 
    : EmpireException("ERR_MAP: " + msg) {}

CombatException::CombatException(const std::string& msg) 
    : EmpireException("ERR_COMBAT: " + msg) {}