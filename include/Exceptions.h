#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>
/*
// Clasa de bază pentru excepțiile imperiului
class EmpireException : public std::runtime_error {
public:
    explicit EmpireException(const std::string& msg);
};

// Excepție pentru lipsa resurselor financiare
class GoldException : public EmpireException {
public:
    explicit GoldException(const std::string& msg);
};

// Excepție pentru atingerea limitei de trupe
class PopulationLimitException : public EmpireException {
public:
    explicit PopulationLimitException(const std::string& msg);
};

// Excepție pentru mișcări invalide pe hartă
class MapException : public EmpireException {
public:
    explicit MapException(const std::string& msg);
};

// Excepție pentru acțiuni invalide în luptă
class CombatException : public EmpireException {
public:
    explicit CombatException(const std::string& msg);
};*/
class EmpireException : public std::runtime_error {
public:
    explicit EmpireException(const std::string& msg);
};

class InsufficientGoldException : public EmpireException {
public:
    InsufficientGoldException();
};

class PopulationLimitException : public EmpireException {
public:
    explicit PopulationLimitException(const std::string& msg);
};

class CombatException : public EmpireException {
public:
    explicit CombatException(const std::string& msg);
};

class InvalidMovementException : public EmpireException {
public:
    explicit InvalidMovementException(const std::string& msg);
};

#endif // EXCEPTIONS_H