#pragma once

#include "API/client.hpp"
#include "API/server.hpp"

namespace client 
{

struct Tank {
    Vec2f pos;
    api::Dir dir;
    Vec2f hitbox_sz;
    // delay of shooting
    // hp
    Tank() = default;
};

class ClientWorld {
    uint64_t tick_;
    api::GameMap map_;
    std::vector<Tank> tanks_;
public:
    void apply_game_state(api::GameState &state) {
        tick_ = state.tick;
        map_ = state.map;
        
        tanks_.clear();
        std::transform(state.tanks.begin(), state.tanks.end(), 
            std::back_inserter(tanks_),
            [&state](auto &tank_info) { 
                Tank tank;
                tank.pos = Vec2f(tank_info.pos.x * state.map.tile_sz, tank_info.pos.y * state.map.tile_sz);
                tank.dir = tank_info.dir;
                tank.hitbox_sz = Vec2f(tank_info.hitbox_sz.x, tank_info.hitbox_sz.y);
                return tank;
            });
    }

    uint64_t tick() const { return tick_; }
    const api::GameMap &map() const { return map_; }
    const std::vector<Tank> &tanks() const { return tanks_; }
};




} // namespace client 