#ifndef GLOBAL_ENUMS_HPP
#define GLOBAL_ENUMS_HPP

// ==========================================================
// 1. ENUMS ȘI CONSTANTE GLOBALE
// ==========================================================

enum class UnitType { 
    INFANTERIE, 
    ARCASI, 
    CAVALERIE, 
    GARDA, 
    EROU 
};

enum class TerrainType { 
    PLAIN, 
    FOREST, 
    MOUNTAIN, 
    WATER, 
    CITY_TILE 
};

enum class GameState { 
    LOGIN, 
    SIMULATION, 
    VICTORIE, 
    DEFEAT 
};

#endif // GLOBAL_ENUMS_HPP
