#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include <utility>
#include <exception>

// Tipuri de teren pentru Grid
enum class TileType { GRASS, FOREST, CITY, MOUNTAIN, WATER };

// Exceptie custom (Cerință Tema 2)
class EmpireException : public std::exception {
protected:
    std::string message;
public:
    explicit EmpireException(std::string msg) : message(std::move(msg)) {}
    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }
};

class InsufficientResourcesException : public EmpireException {
public:
    explicit InsufficientResourcesException(const std::string& res)
        : EmpireException("Resurse insuficiente: " + res) {}
};

#endif