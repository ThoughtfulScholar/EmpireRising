// #include "Exceptions.h"
#include "../include/Exceptions.h"
/*
EmpireException::EmpireException(const std::string& msg) 
    : std::runtime_error(msg) {}

InsufficientGoldException::GoldException(const std::string& msg)
    : EmpireException("Tezaur insuficient!") {}

PopulationLimitException::PopulationLimitException(const std::string& msg) 
    : EmpireException("ERR_POPULATION: " + msg) {}

MapException::MapException(const std::string& msg) 
    : EmpireException("ERR_MAP: " + msg) {}

CombatException::CombatException(const std::string& msg) 
    : EmpireException("ERR_COMBAT: " + msg) {}*/

EmpireException::EmpireException(const std::string& msg)
    : std::runtime_error(msg) {}

// InsufficientGoldException trimite un mesaj predefinit către EmpireException
InsufficientGoldException::InsufficientGoldException()
    : EmpireException("Tezaur insuficient!") {}

PopulationLimitException::PopulationLimitException(const std::string& msg)
    : EmpireException(msg) {}

CombatException::CombatException(const std::string& msg)
    : EmpireException(msg) {}

InvalidMovementException::InvalidMovementException(const std::string& msg)
    : EmpireException(msg) {}