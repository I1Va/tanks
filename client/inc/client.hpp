#pragma once

#include <iostream>
#include <exception>
#include <memory>
#include <string>

#include <SDL2/SDL_image.h>

#include "SDLRAII.hpp"
#include "err_proc.hpp"
#include "SDL_utility.hpp"
#include "API/client.hpp"
#include "API/server.hpp"

namespace client 
{

class ClientWorld {
    uint64_t tick_;
    api::GameMap map_;
public:
    void apply_game_state(api::GameState &state) {
        tick_ = state.tick;
        map_ = state.map;
    }

    uint64_t tick() const { return tick_; }
    const api::GameMap &map() const { return map_; }
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
public:
    struct TexturePack {
        std::string wall_texture_path="assets/textures/bricks/Brick.png";
        std::string empty_texture_path="assets/textures/bricks/Grey bricks.png";      
    };

    struct Config {
        int screen_width=800;
        int screen_height=600;
        int tile_sz=50;

        std::string font_path="/usr/share/fonts/TTF/Hack-Bold.ttf";
        int font_size=24;

        TexturePack texture_pack;
    };

private:
    Config config_;

    SDL_Init_Guard sdl_init;
    TTF_SDL_Init_Guard ttf_init;

    raii::SDL_Window window_=nullptr;
    raii::SDL_Renderer renderer_=nullptr;
    raii::TTF_Font font_=nullptr;

public:
    Graphics(const Config &config) : config_(config) {
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
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

        // TODO: fix perforamnce. Load texture ones in initialization;
    
        for (const auto &line : world.map().grid) {
            for (const auto &tile : line) {
                SDL_Texture *texture=nullptr;
                std::string texture_path;
                switch (tile.type) {
                    case api::Tile::Type::WALL: texture_path = config_.texture_pack.wall_texture_path; break;
                    case api::Tile::Type::EMPTY: texture_path = config_.texture_pack.empty_texture_path; break;
                    default: assert("invalid tile type");
                }

                texture = loadTexture(renderer_.get(), texture_path);
                if (!texture) throw SDLException();

                SDL_Rect dstRect = {tile.cord.x * config_.tile_sz, tile.cord.y * config_.tile_sz, config_.tile_sz, config_.tile_sz};
                SDL_RenderCopy(renderer_.get(), texture, nullptr, &dstRect);

            }
        }
        SDL_RenderPresent(renderer_.get());
    }

    void show() {
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
