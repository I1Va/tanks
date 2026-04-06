#pragma once

#include <iostream>
#include <exception>
#include <memory>

#include "SDLRAII.hpp"
#include "err_proc.hpp"
#include "API/client.hpp"
#include "API/server.hpp"

namespace client 
{

class ClientWorld {
    uint64_t tick_;
public:
    void apply_game_state(api::GameState &state) {
        tick_ = state.tick;
    }

    uint64_t tick() const { return tick_; }
};

// class Net {
// public:
//     Net() = default;

//     bool connect(IServer &server) {

//     }

//     // receives raw bytes from server 

// };

struct TTF_SDL_Init_Guard {
    TTF_SDL_Init_Guard() { if (TTF_Init()!=0) throw TTFException(); }
    ~TTF_SDL_Init_Guard() { TTF_Quit(); }
};

struct SDL_Init_Guard {
    SDL_Init_Guard() { if (SDL_Init(SDL_INIT_VIDEO)!=0) throw SDLException(); }
    ~SDL_Init_Guard() { TTF_Quit(); }
};


class Graphics {
    SDL_Init_Guard sdl_init;
    TTF_SDL_Init_Guard ttf_init;

    raii::SDL_Window window_=nullptr;
    raii::SDL_Renderer renderer_=nullptr;
    raii::TTF_Font font_=nullptr;

public:
    struct Config {
        size_t screen_width=800;
        size_t screen_height=600;
        std::string font_path="/usr/share/fonts/TTF/Hack-Bold.ttf";
        size_t font_size=24;
    };

    Graphics(const Config &config) {
        SDL_Window * window_ptr = 
        SDL_CreateWindow (
            "tanks game",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.screen_width, config.screen_height,
            0
        );
        if (!window_ptr) throw SDLException();

        window_.reset(window_ptr);

        SDL_Renderer *renderer_ptr = SDL_CreateRenderer(
            window_.get(), -1, SDL_RENDERER_ACCELERATED
        );
        if (!renderer_ptr)  throw SDLException();

        renderer_.reset(renderer_ptr);

        TTF_Font *font_ptr = TTF_OpenFont(config.font_path.c_str(), config.font_size); 
        if (!font_ptr) throw TTFException();
        font_.reset(font_ptr); 
    }

    ~Graphics() {}

    void render(const ClientWorld &world) {
        // clear screen
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        // prepare tick text
        std::string tick_text = "Tick: " + std::to_string(world.tick());
        SDL_Color color = {255, 255, 255, 255}; // white

        SDL_Surface* surface = TTF_RenderText_Solid(font_.get(), tick_text.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_.get(), surface);
        SDL_FreeSurface(surface);

        SDL_Rect dstRect = {10, 10, 200, 50}; // position and size
        SDL_RenderCopy(renderer_.get(), texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);

        // present everything
        SDL_RenderPresent(renderer_.get());
    }

    void show() {
        SDL_RenderClear(renderer_.get());
        SDL_RenderPresent(renderer_.get());
    }
};

class Client : public api::IClient { 
    Graphics graphics_;
    ClientWorld world_;

public:
    struct Config {
        Graphics::Config gfx_config;
    };

    Client(const Config &config): graphics_(config.gfx_config) {}

    void show() { graphics_.show(); }

    api::Input sendInput() const override {
        return {};
    }

    // receive game state from server
    void receive(api::GameState &state) override {
        world_.apply_game_state(state);
        graphics_.render(world_);
    }

    bool connect(const std::string &ip, int port) override {
        return true;
    }

    bool isConnected() const override {
        return true;
    }
};


} // namespace client
