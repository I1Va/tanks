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

    std::map<uint64_t, Tank> tanks_; 
    api::TankId tank_id_=0;

public:
    ServerWorld(const api::GameMap &map): map_(map) {}

    void simulate_step(float dt) {
        // TODO: add queue of events and process them there  
        ++tick_;
    }
    
    api::TankId get_new_tank_id() { return tank_id_++; }
    
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

    void spawn_tank_in_tile(const api::Cord tile_pos, const api::TankId id) {
        tanks_[id] = Tank();
        auto &tank = tanks_[id];
        tank.pos = tile_pos;
        tank.hitbox_sz = get_tank_hitbox_size();
    }

    void tank_rotate(const api::TankId tank_id, api::Dir dir) {
        if (!tanks_.contains(tank_id)) return;
        tanks_[tank_id].dir = dir;
    }

    api::Cord get_tank_hitbox_size() const {
        int sz = map_.tile_sz * 0.9; 
        return {sz, sz};
    }

    void tank_move_forward(const api::TankId tank_id) {
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
        if (!tanks_.contains(id)) return 1;
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


struct ServerCommand {
    enum Type { 
        MoveForward, 
        Rotate,
        Spawn,
    };
    Type type;
    api::TankId tank_id;
    api::Dir dir; 
    api::Cord tile_pos;
};

class Server : public api::IServer {   
    ServerWorld world_;
    std::vector<api::IClient *> clients_;
    std::queue<ServerCommand> comand_queue;
    
public:
    ~Server() = default;
    Server(const api::GameMap &map): world_(map) {}

    void add_client(api::IClient *client) override {
        assert(client);
        clients_.push_back(client);
    }

    void update() override {
        execute_command_queue();
    
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
        ServerCommand comand;
        api::TankId id = world_.get_new_tank_id();
        comand.type = ServerCommand::Type::Spawn;
        comand.tank_id = id;
        comand.tile_pos = tile_pos;
        comand_queue.push(comand);
        return id;
    }

    void move_torward(const api::TankId tank_id) override {
        ServerCommand comand;
        comand.type = ServerCommand::Type::MoveForward;
        comand.tank_id = tank_id;
        comand_queue.push(comand);
    }
    
    void rotate(const api::TankId tank_id, const api::Dir dir) override {
        ServerCommand comand;
        comand.type = ServerCommand::Type::Rotate;
        comand.tank_id = tank_id;
        comand.dir = dir;
        comand_queue.push(comand);
    }

    int get_tank_info(api::TankId id, api::TankInfo &info) {
        return world_.get_tank_info(id, info);
    }

private:
    void execute_command_queue() {
        while (!comand_queue.empty()) {
            auto &command = comand_queue.front();
            switch (command.type) {
                case ServerCommand::Type::Rotate: 
                    world_.tank_rotate(command.tank_id, command.dir); break;
                case ServerCommand::Type::MoveForward: 
                    world_.tank_move_forward(command.tank_id); break;
                case ServerCommand::Type::Spawn:
                    world_.spawn_tank_in_tile(command.tile_pos, command.tank_id); break;
                default:
                    assert(false && "unknown command in queue" && (int) command.type);
            }
            comand_queue.pop();
        }   
    }
};

}; // namespace server

