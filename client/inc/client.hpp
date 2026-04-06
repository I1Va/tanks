#pragma once

#include <iostream>
#include <exception>
#include <memory>

#include "SDLRAII.hpp"
#include "server_api/api.hpp"

namespace client 
{

class GameState {
    uint64_t tick_;
};

class Net {
public:
    Net() = default;

    // receives raw bytes from server 

};


class Graphics {
    raii::SDL_Window window_=nullptr;
    raii::SDL_Renderer renderer_=nullptr;

public:
    struct Config {
        size_t screen_width=800;
        size_t screen_height=600;   
    };

    Graphics(const Config &config) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("Failed to init SDL");
        }

        SDL_Window * window_ptr = 
        SDL_CreateWindow (
            "tanks game",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.screen_width, config.screen_height,
            0
        );
        if (!window_ptr) throw std::runtime_error("Failed to create SDL_Window");

        window_.reset(window_ptr);

        SDL_Renderer *renderer_ptr = SDL_CreateRenderer(
            window_.get(), -1, SDL_RENDERER_ACCELERATED
        );
        if (!renderer_ptr) throw std::runtime_error("Failed to create SDL_Renderer");

        renderer_.reset(renderer_ptr);
    }

    ~Graphics() {
        SDL_Quit();
    }

    void show() {
        SDL_RenderClear(renderer_.get());
        SDL_RenderPresent(renderer_.get());
    }
};


class TanksGame { 
    Graphics graphics_;

public:
    struct Config {
        Graphics::Config gfx_config;
    };

    TanksGame(const Config &config): graphics_(config.gfx_config) {}

    void show() {
        graphics_.show();
    }
};


} // namespace client
