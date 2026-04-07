#pragma once

#include <chrono>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>

#include "API/server.hpp"
#include "API/client.hpp"

namespace server {

class Tank : public api::ITank {
    api::Cord hitbox_size_={};
    api::Cord pos_={};
    api::ITank::Dir dir_=api::ITank::Dir::UP;

public:
    Tank(api::Cord hitbox_size): hitbox_size_(hitbox_size) {}
    
    void set_pos(const api::Cord pos) override { pos_ = pos; }
    api::Cord get_pos() const override { return pos_; } 
    void set_dir(const api::ITank::Dir dir) { dir_ = dir; }
    api::Cord get_hitbox_size() const override { return hitbox_size_; }    
    api::ITank::Dir get_dir() const override { return dir_; }
};

class ServerWorld { // GameWorld logic
    api::GameMap map_;
    uint64_t tick_=0;
    std::map<const api::ITank *, std::unique_ptr<api::ITank>> tanks_;

public:
    ServerWorld(const api::GameMap &map): map_(map) {}

    void simulate_step(float dt) {
        ++tick_;
    }
    
    api::GameState create_snapshot() const {
        api::GameState state;
        state.map = map_;
        state.tick = tick_;
        std::transform(tanks_.begin(), tanks_.end(), 
            std::back_inserter(state.tanks),
            [](auto &tank_pair) { return tank_pair.second.get(); });

        return state;
    }

    void apply_input(const api::Input &input) {

    }

    api::ITank *spawn_tank_in_tile(const api::Cord tile_pos) {
        auto tank = std::make_unique<Tank>(get_tank_hitbox_size());
        auto tank_ptr = tank.get();
        api::Cord pos = {
            static_cast<int>(tile_pos.x * map_.tile_sz), 
            static_cast<int>(tile_pos.y * map_.tile_sz)
        };
        tank->set_pos(pos);
        tanks_[tank_ptr] = std::move(tank);    
      
        return tank_ptr;
    }

    void rotate(const api::ITank *tank, api::ITank::Dir dir) {
        assert(tank);
        // assert(tanks_.contains(tank));
        tanks_[tank]->set_dir(dir);
    }

    api::Cord get_tank_hitbox_size() const {
        int sz = map_.tile_sz * 0.9; 
        return {sz, sz};
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

    api::ITank *spawn_tank_in_tile(const api::Cord tile_pos) override {
        return world_.spawn_tank_in_tile(tile_pos);
    }

    void move_torward(const api::ITank *tank) override {

    }

    void rotate(const api::ITank *tank, api::ITank::Dir dir) override {
        world_.rotate(tank, dir);
    }
};

}; // namespace server

