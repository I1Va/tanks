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
    // delay of shooting
    // hp
    Tank() = default;

    api::TankInfo get_info() {
        api::TankInfo info;
        info.pos = pos;
        info.dir = dir;
        return info;
    }
};

struct Bullet {
    static constexpr int BULLET_SPEED = 1;

    api::Cord pos;
    api::Dir dir;
    int speed;           
    api::TankId owner;


    Bullet() = default;

    api::BulletInfo get_info() const {
        api::BulletInfo info;
        info.pos = pos;
        info.dir = dir;
        info.speed = speed;
        info.owner = owner;
        return info;
    }
};


class ServerWorld { // GameWorld logic
 
private:
    api::GameMap map_;
    uint64_t tick_=0;

    std::map<uint64_t, Tank> tanks_; 
    std::vector<Bullet> bullets_;
    api::TankId tank_id_=0;

public:
    ServerWorld(const api::GameMap &map): map_(map) {}
    
    api::TankId get_new_tank_id() { return tank_id_++; }
    
    api::GameState create_snapshot() const {
        api::GameState state;
        state.map = map_;
        state.tick = tick_;
        std::transform(tanks_.begin(), tanks_.end(), 
            std::back_inserter(state.tanks),
            [](auto tank_pair) { return tank_pair.second.get_info(); });

        std::transform(bullets_.begin(), bullets_.end(), 
            std::back_inserter(state.bullets),
            [](auto &bullet) { return bullet.get_info(); });

        return state;
    }

    void spawn_tank_in_tile(const api::Cord tile_pos, const api::TankId id) {
        tanks_[id] = Tank();
        auto &tank = tanks_[id];
        tank.pos = tile_pos;
    }

    void tank_rotate(const api::TankId tank_id, api::RotationDir rot_dir) {
        if (!tanks_.contains(tank_id)) return;

        tanks_[tank_id].dir = get_rotated_dir(tanks_[tank_id].dir, rot_dir); 
    }

    void turret_fire(const api::TankId tank_id) {
        if (!tanks_.contains(tank_id)) return;

        Tank &tank = tanks_[tank_id];

        Bullet bullet;
        bullet.pos = tank.pos;
        bullet.dir = tank.dir;
        bullet.speed = Bullet::BULLET_SPEED;
        bullet.owner = tank_id;

        bullets_.push_back(bullet);
    }

     void simulate_step() {
        for (auto it = bullets_.begin(); it != bullets_.end(); ) {
            api::Cord delta = dir_to_cord(it->dir);
            it->pos.x += delta.x * it->speed;
            it->pos.y += delta.y * it->speed;

            if (is_wall(it->pos)) {
                it = bullets_.erase(it);
                continue;
            }
            bool hit = false;
            for (auto &[id, tank] : tanks_) {
                if (id == it->owner) continue;
                if (tank.pos == it->pos) {
                    it = bullets_.erase(it);
                    tanks_.erase(id);   // tank destroyed
                    hit = true;
                    break;
                }
            }
        if (hit) continue;
        ++it;
        }
    }

    bool is_wall(const api::Cord cord) {
        if (cord.y >= (int) map_.grid.size()) return false;
        if (cord.x >= (int) map_.grid[0].size()) return false;
        if (cord.y < 0) return false;
        if (cord.x < 0) return false;
        return map_.grid[cord.y][cord.x].type == api::Tile::Type::Wall;
    }

    api::Cord get_tank_hitbox_size() const {
        int sz = map_.tile_sz * 0.9; 
        return {sz, sz};
    }

     bool is_tile_occupied(const api::Cord &cord, api::TankId exclude_id = 0) const {
        for (const auto &[id, tank] : tanks_) {
            if (id == exclude_id) continue;
            if (tank.pos == cord) return true;
        }
        return false;
    }

    void tank_move_forward(const api::TankId tank_id) {
        if (!tanks_.contains(tank_id)) return;

        auto &tank = tanks_[tank_id];
        api::Dir dir = tank.dir;
        api::Cord pos = tank.pos;
        api::Cord dir_vec = dir_to_cord(dir);

        api::Cord new_pos = {
            static_cast<int>(pos.x + dir_vec.x),
            static_cast<int>(pos.y + dir_vec.y)
        };

        if (is_wall(new_pos)) return;
        if (is_tile_occupied(new_pos, tank_id)) return;

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
        TankRotate,
        Spawn,
        TurretFire,
    };
    Type type;
    api::TankId tank_id;
    api::RotationDir rot_dir; 
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
        world_.simulate_step();        

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

    void tank_move_torward(const api::TankId tank_id) override {
        ServerCommand comand;
        comand.type = ServerCommand::Type::MoveForward;
        comand.tank_id = tank_id;
        comand_queue.push(comand);
    }
    
    void tank_rotate(const api::TankId tank_id, const api::RotationDir dir) override {
        ServerCommand comand;
        comand.type = ServerCommand::Type::TankRotate;
        comand.tank_id = tank_id;
        comand.rot_dir = dir;
        comand_queue.push(comand);
    }

    void turret_fire(const api::TankId tank_id) override {
        ServerCommand comand;
        comand.type = ServerCommand::Type::TurretFire;
        comand.tank_id = tank_id;
        comand_queue.push(comand);
    }    

    int get_tank_info(api::TankId id, api::TankInfo &info) override {
        return world_.get_tank_info(id, info);
    }

private:
    void execute_command_queue() {
        while (!comand_queue.empty()) {
            auto &command = comand_queue.front();
            switch (command.type) {
                case ServerCommand::Type::MoveForward: world_.tank_move_forward(command.tank_id); break;
                case ServerCommand::Type::TankRotate: world_.tank_rotate(command.tank_id, command.rot_dir); break;
                case ServerCommand::Type::TurretFire: world_.turret_fire(command.tank_id); break;
                case ServerCommand::Type::Spawn: world_.spawn_tank_in_tile(command.tile_pos, command.tank_id); break;
                default:
                    assert(false && "unknown command in queue" && (int) command.type);
            }
            comand_queue.pop();
        }   
    }
};

}; // namespace server

