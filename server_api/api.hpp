#pragma once

#include <vector>
#include <cstdint>

struct Cord {
    int x;
    int y;
};

struct Tile {
    enum class Type {
        WALL,
        EMPTY,
    };
    Type type;
    Cord cord;
};

struct GameMap {
    std::vector<std::vector<Tile>> grid;
    GameMap(const size_t height, const size_t width): grid(height, std::vector<Tile>(width)) {}
};

struct WorldSnapshot { // game data for client sending 
    GameMap map;
    uint64_t tick; 
    // std::vector<TankState> tanks;
    // std::vector<ProjectileState> projectiles;
};

// class IServer {
// }


// TODO: make abstract server API class