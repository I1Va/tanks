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
    std::vector<api::ITank *> tanks_;
public:
    void apply_game_state(api::GameState &state) {
        tick_ = state.tick;
        map_ = state.map;
        tanks_ = state.tanks;
    }

    uint64_t tick() const { return tick_; }
    const api::GameMap &map() const { return map_; }
    const std::vector<api::ITank *> &tanks() const { return tanks_; }
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
    struct TexturePackPathes {
        std::string wall_texture_path="assets/textures/bricks/Brick.png";
        std::string empty_texture_path="assets/textures/bricks/Greybricks.png";      
    };

    struct Config {
        int screen_width=600;
        int screen_height=600;
        int tile_sz=60;

        std::string font_path="/usr/share/fonts/TTF/Hack-Bold.ttf";
        int font_size=24;

        TexturePackPathes texture_pack_pathes;
    };

    struct TexturePack {
        raii::SDL_Texture wall_tile_texture=nullptr;
        raii::SDL_Texture empty_tile_texture=nullptr;

        void load(SDL_Renderer *renderer, const TexturePackPathes &pathes) {
            SDL_Texture *texture=nullptr;
            requireSDLCondition(texture=load_texture(renderer, pathes.wall_texture_path));
            wall_tile_texture.reset(texture);

            requireSDLCondition(texture=load_texture(renderer, pathes.empty_texture_path));
            empty_tile_texture.reset(texture);
        }
    };

private:
    Config config_;

    SDL_Init_Guard sdl_init;
    TTF_SDL_Init_Guard ttf_init;

    raii::SDL_Window window_=nullptr;
    raii::SDL_Renderer renderer_=nullptr;
    raii::TTF_Font font_=nullptr;

    TexturePack texture_pack_;
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

        texture_pack_.load(renderer_.get(), config_.texture_pack_pathes);
    }

    ~Graphics() {}

    void render(const ClientWorld &world) {
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());
        
        const std::vector<std::vector<api::Tile>> &grid = world.map().grid;
        for (size_t y = 0; y < grid.size(); y++) {
            for (size_t x = 0; x < world.map().grid[y].size(); x++) {
                int w = config_.tile_sz;
                int h = config_.tile_sz;
                api::Tile tile = grid[y][x];
                switch (tile.type) {
                    case api::Tile::Type::WALL:  draw_texture(renderer_.get(), texture_pack_.wall_tile_texture.get(), x * w, y * h, w, h); break;
                    case api::Tile::Type::EMPTY: draw_texture(renderer_.get(), texture_pack_.empty_tile_texture.get(), x * w, y * h, w, h); break;
                    default: 
                    assert("invalid tile type");
                }
            }
        }

        for (auto tank : world.tanks()) {
            SDL_Rect rect = {tank->get_pos().x, tank->get_pos().y, 50, 50};

            SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 255, 255); 
            SDL_RenderDrawRect(renderer_.get(), &rect);
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
