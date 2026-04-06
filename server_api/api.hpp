#pragma once

#include <vector>
#include <cstdint>

struct HexCord {
    int q;
    int r;
};

struct HexTile {
    enum class Type {
        FOREST,
        LAKE,
        MOUNTAIN,
    };
    Type type;
    HexCord cord;
};

class GameMap {
    std::vector<std::vector<HexTile>> grid_;
public:
    GameMap(const size_t height, const size_t width): grid_(height, std::vector<HexTile>(width)) {}

    HexTile& get(const HexCord cord) {
        return grid_[cord.r][cord.q + (cord.r >> 1)];
    }
};

struct WorldSnapshot { // game data for client sending 
    GameMap map;
    uint64_t tick; 
    // std::vector<TankState> tanks;
    // std::vector<ProjectileState> projectiles;
};


// TODO: make abstract server API class