#pragma once

#include <chrono>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>

#include "API/server.hpp"
#include "API/client.hpp"

namespace server {

// class IServerEvent {

// };

// struct TankMoveServerEvent : IServerEvent {
    
// };

struct Tank {
    api::Cord pos;
    api::Dir dir;
    api::Cord hitbox_sz;
    // delay of shooting
    // hp
    Tank() = default;

    api::TankInfo get_info() {
        api::TankInfo info;
        info.pos = pos;
        info.dir = dir;
        info.hitbox_sz = hitbox_sz;
        return info;
    }
};

class ServerWorld { // GameWorld logic
 
private:
    api::GameMap map_;
    uint64_t tick_=0;

    api::TankId tank_id=0;
    std::map<uint64_t, Tank> tanks_; 

public:
    ServerWorld(const api::GameMap &map): map_(map) {}

    void simulate_step(float dt) {
        // TODO: add queue of events and process them there  
        ++tick_;
    }
    
    api::GameState create_snapshot() const {
        api::GameState state;
        state.map = map_;
        state.tick = tick_;
        std::transform(tanks_.begin(), tanks_.end(), 
            std::back_inserter(state.tanks),
            [](auto tank_pair) { return tank_pair.second.get_info(); });

        return state;
    }

    void apply_input(const api::Input &input) {

    }

    uint64_t spawn_tank_in_tile(const api::Cord tile_pos) {
        tanks_[tank_id] = Tank();
        auto &tank = tanks_[tank_id];
        tank.pos = tile_pos;
        tank.hitbox_sz = get_tank_hitbox_size();
        return tank_id++;
    }

    void rotate(const api::TankId tank_id, api::Dir dir) {
        if (!tanks_.contains(tank_id)) return;
        tanks_[tank_id].dir = dir;
    }

    api::Cord get_tank_hitbox_size() const {
        int sz = map_.tile_sz * 0.9; 
        return {sz, sz};
    }

    void move_torward(const api::TankId tank_id) {
        if (!tanks_.contains(tank_id)) return;

        // TOOD: process walls, tanks collision
        // TODO: add simulate_step synchronization
        
        auto &tank = tanks_[tank_id];

        api::Dir dir = tank.dir;
        api::Cord pos = tank.pos;
        api::Cord dir_vec = dir_to_cord(dir);

        api::Cord new_pos = {
            static_cast<int>(pos.x + dir_vec.x),
            static_cast<int>(pos.y + dir_vec.y)
        };
        
        tank.pos = new_pos;
    }    

    int get_tank_info(api::TankId id, api::TankInfo &info) {
        if (tanks_.contains(id)) return 1;
        info = tanks_[id].get_info();
        return 0;
    }

private:
    static api::Cord dir_to_cord(api::Dir dir) {
    switch (dir) {
            case api::Dir::UP:    return { 0, -1 }; // move up
            case api::Dir::DOWN:  return { 0,  1 }; // move down
            case api::Dir::LEFT:  return { -1, 0 }; // move left
            case api::Dir::RIGHT: return { 1,  0 }; // move right
            default: return {0, 0}; // no movement
        }
    }
};

class Server : public api::IServer {   
    ServerWorld world_;
    std::vector<api::IClient *> clients_;
public:
    ~Server() = default;
    Server(const api::GameMap &map): world_(map) {}

    void add_client(api::IClient *client) override {
        assert(client);
        clients_.push_back(client);
    }

    void update() override {
        const float TICK_RATE = 30.0f;
        
        std::for_each(clients_.begin(), clients_.end(), 
            [this](api::IClient* client) {
            api::Input client_input = client->sendInput();
            world_.apply_input(client_input);
        });

        world_.simulate_step(TICK_RATE);
    
        std::for_each(clients_.begin(), clients_.end(), 
            [this](api::IClient* client) {
                api::GameState snapshot = world_.create_snapshot();
                client->receive(snapshot);
        });
    }

    api::TankId spawn_tank_in_tile(const api::Cord tile_pos) override {
        return world_.spawn_tank_in_tile(tile_pos);
    }

    void move_torward(const api::TankId tank_id) override {
        world_.move_torward(tank_id);
    }
    void rotate(const api::TankId tank_id, const api::Dir dir) override {
        world_.rotate(tank_id, dir);
    }
    int get_tank_info(api::TankId id, api::TankInfo &info) {
        return world_.get_tank_info(id, info);
    }
};

}; // namespace server

