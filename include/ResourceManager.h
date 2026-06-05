// include/ResourceManager.h
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <stdexcept>
#include "Exceptions.h" // Presupunem că aici ai PopulationLimitException

template <typename T, int Capacity>
class ResourceManager {
private:
    std::vector<T> items;

public:
    ResourceManager() = default;

    void add(T item) {
        if (items.size() >= Capacity) {
            // Folosește cu sens excepțiile tale din Tema 2!
            throw PopulationLimitException("Capacitatea maxima a managerului a fost atinsa (" + std::to_string(Capacity) + ")!");
        }
        items.push_back(std::move(item));
    }

    //const std::vector<T>& getItems() const { return items; }
    //size_t getSize() const { return items.size(); }
    //int getCapacity() const { return Capacity; }
};

#endif // RESOURCEMANAGER_H