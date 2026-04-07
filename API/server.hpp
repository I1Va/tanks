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
    Type type= Type::EMPTY;
};

struct GameMap {
    std::vector<std::vector<Tile>> grid;
    GameMap(const size_t height, const size_t width): grid(height, std::vector<Tile>(width)) {}
    GameMap() = default;
};

class ITank {
public:
    enum class Dirs {
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    virtual ~ITank() = default;
    virtual void set_pos(const api::Cord pos) = 0;
    virtual api::Cord get_pos() const = 0;     
};

struct GameState {
    GameMap map;
    uint64_t tick; 
    std::vector<api::ITank *> tanks;
    // std::vector<TankState> tanks;
    // std::vector<ProjectileState> projectiles;
};


class IServer {
public:
    virtual ~IServer() = default;
    virtual void add_client(IClient *client) = 0;
    virtual void update() = 0;

    virtual ITank *spawn_tank(const Cord pos) = 0;

    // virtual void move(const ITank *tank, ITank::Dirs dir, float dt) = 0;
    // virtual void rotate(const ITank *tank, ITank::Dirs dir) = 0;    
};

} // namespace api

