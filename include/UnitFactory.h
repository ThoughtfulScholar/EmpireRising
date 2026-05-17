#ifndef UNITFACTORY_H
#define UNITFACTORY_H

#include "Unit.h"
#include "Enums.h"
#include "Exceptions.h"
#include <memory>
#include <string>

// Fabrica centralizată de unități militare

struct UnitFactory {
    static std::unique_ptr<Unit> CreateUnit(UnitType type, const std::string& customName = "");
};

#endif // UNITFACTORY_H