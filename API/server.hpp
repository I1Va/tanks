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
    size_t tile_sz=0;
    GameMap
    (
        const size_t height, const size_t width,
        const size_t itile_sz
    ): 
    grid(height, std::vector<Tile>(width)),
    tile_sz(itile_sz)
    {}

    GameMap()=default;
};

using TankId = uint64_t;

enum class Dir {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum class RotationDir {
    LEFT,
    RIGHT,
};

inline api::Dir get_next_dir(const api::Dir dir) {
    switch (dir) {
        case api::Dir::UP:    return api::Dir::RIGHT;
        case api::Dir::RIGHT: return api::Dir::DOWN;
        case api::Dir::DOWN:  return api::Dir::LEFT;
        case api::Dir::LEFT:  return api::Dir::UP;
        default: return api::Dir::UP;
    }
}

inline api::Dir get_prev_dir(const api::Dir dir) {
    switch (dir) {
        case api::Dir::UP:    return api::Dir::LEFT;
        case api::Dir::RIGHT: return api::Dir::UP;
        case api::Dir::DOWN:  return api::Dir::RIGHT;
        case api::Dir::LEFT:  return api::Dir::DOWN;
        default: return api::Dir::UP;
    }
}

inline api::Dir get_rotated_dir(const api::Dir dir, const api::RotationDir rot_dir) {
    if (rot_dir == api::RotationDir::LEFT) {
        return get_prev_dir(dir);
    } else {
        return get_next_dir(dir);
    }   
} 

struct TankInfo {
    Cord pos;
    Dir dir;
    Dir turret_dir;
    Cord hitbox_sz;
};

struct GameState {
    GameMap map;
    uint64_t tick; 
    std::vector<TankInfo> tanks;
};

class IServer {
public:
    virtual ~IServer() = default;
    virtual void add_client(IClient *client) = 0;
    virtual void update() = 0;

    virtual TankId spawn_tank_in_tile(const Cord tile_pos) = 0;
    virtual void tank_move_torward(const TankId tank_id) = 0;
    virtual void tank_rotate(const TankId tank_id, const RotationDir dir) = 0;
    virtual void turret_rotate(const TankId tank_id, const RotationDir dir) = 0;
    virtual void turret_fire(const TankId tank_id) = 0;
    virtual int get_tank_info(TankId id, TankInfo &info) = 0;
};

} // namespace api

