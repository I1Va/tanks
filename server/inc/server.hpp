#pragma once

#include <chrono>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>

#include "API/server.hpp"
#include "API/client.hpp"

namespace server {

class ServerWorld { // GameWorld logic
    api::GameMap map_;
    uint64_t tick_=0;
public:
    ServerWorld(const api::GameMap &map): map_(map) {}

    void simulate_step(float dt) {
        ++tick_;
    }
    
    api::GameState create_snapshot() const {
        return api::GameState{map_, tick_};
    }

    void apply_input(const api::Input &input) {

    }
};

class Server : public api::IServer {   
    ServerWorld world_;
    std::vector<api::IClient *> clients_;
public:
    ~Server() = default;
    Server(const ServerWorld &world): world_(world) {}

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
};

}; // namespace server

