#ifndef UNIT_TYPES_HPP
#define UNIT_TYPES_HPP

#include "Unit.hpp"

class Infantry : public Unit {
public:
    Infantry();
    std::unique_ptr<Unit> clone() const override;
};

class Archer : public Unit {
public:
    Archer();
    std::unique_ptr<Unit> clone() const override;
};

class Cavalry : public Unit {
public:
    Cavalry();
    std::unique_ptr<Unit> clone() const override;
};

class GarrisonGuard : public Unit {
public:
    explicit GarrisonGuard(const std::string& n);
    std::unique_ptr<Unit> clone() const override;
};

class Hero : public Unit {
public:
    Hero(const std::string& n, int h, int a, int u);
    std::unique_ptr<Unit> clone() const override;
};

class GarrisonUnit : public Unit {
public:
    GarrisonUnit();
    std::unique_ptr<Unit> clone() const override;
};

#endif
