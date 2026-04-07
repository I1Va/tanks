#pragma once

#include <iostream>
#include <exception>
#include <memory>
#include <string>

#include "API/client.hpp"
#include "API/server.hpp"
#include "vec2.hpp"

#include "graphics.hpp"
#include "client_world.hpp"

namespace client 
{

class Client : public api::IClient { 
    Graphics graphics_;
    ClientWorld world_;

public:
    struct Config {
        Graphics::Config gfx_config;
    };

    Client(const Config &config): graphics_(config.gfx_config) {}

    void update() { 
        graphics_.render(world_); 
    }

    // receive game state from server
    void receive(api::GameState &state) override {
        world_.apply_game_state(state);
    }

    bool connect(const std::string &, int) override {
        return true;
    }

    bool isConnected() const override {
        return true;
    }
};


} // namespace client
