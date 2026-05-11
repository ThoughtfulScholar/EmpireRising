#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

// ==========================================================
// 2. IERARHIA DE EXCEPȚII (Cerință Tema 2)
// ==========================================================

class EmpireException : public std::runtime_error {
public:
    explicit EmpireException(const std::string& msg) : std::runtime_error(msg) {}
};

class InsufficientGoldException : public EmpireException {
public:
    InsufficientGoldException() : EmpireException("Tezaur insuficient!") {}
};

class PopulationLimitException : public EmpireException {
public:
    explicit PopulationLimitException(const std::string& msg) : EmpireException(msg) {}
};

class CombatException : public EmpireException {
public:
    explicit CombatException(const std::string& msg) : EmpireException(msg) {}
};

class InvalidMovementException : public EmpireException {
public:
    explicit InvalidMovementException(const std::string& msg) : EmpireException(msg) {}
};

#endif // EXCEPTIONS_HPP
