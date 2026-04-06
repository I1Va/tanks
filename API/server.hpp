#pragma once

#include <vector>
#include <cstdint>

namespace api {

class IClient;

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
    GameMap() = default;
};

struct GameState {
    GameMap map;
    uint64_t tick; 
    // std::vector<TankState> tanks;
    // std::vector<ProjectileState> projectiles;
};

class IServer {
public:
    virtual ~IServer() = default;
    virtual void add_client(IClient *client) = 0;
    virtual void update() = 0;
};

} // namespace api

