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

struct TankInfo {
    Cord pos;
    Dir dir;
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
    virtual void move_torward(const TankId tank_id) = 0;
    virtual void rotate(const TankId tank_id, const Dir dir) = 0;
    
    virtual int get_tank_info(TankId id, TankInfo &info) = 0;
};

} // namespace api

