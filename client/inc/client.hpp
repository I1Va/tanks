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
#include "vec2.hpp"

namespace client 
{

struct Tank {
    Vec2f pos;
    api::Dir dir;
    Vec2f hitbox_sz;
    // delay of shooting
    // hp
    Tank() = default;
};

class ClientWorld {
    uint64_t tick_;
    api::GameMap map_;
    std::vector<Tank> tanks_;
public:
    void apply_game_state(api::GameState &state) {
        tick_ = state.tick;
        map_ = state.map;
        
        tanks_.clear();
        std::transform(state.tanks.begin(), state.tanks.end(), 
            std::back_inserter(tanks_),
            [&state](auto &tank_info) { 
                Tank tank;
                tank.pos = Vec2f(tank_info.pos.x * state.map.tile_sz, tank_info.pos.y * state.map.tile_sz);
                tank.dir = tank_info.dir;
                tank.hitbox_sz = Vec2f(tank_info.hitbox_sz.x, tank_info.hitbox_sz.y);
                return tank;
            });
    }

    uint64_t tick() const { return tick_; }
    const api::GameMap &map() const { return map_; }
    const std::vector<Tank> &tanks() const { return tanks_; }
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
    ~SDL_Init_Guard() { SDL_Quit(); }
};


class Graphics {
public:
    struct TexturePackPathes {
        std::string wall_texture_path="assets/textures/bricks/Brick.png";
        std::string empty_texture_path="assets/textures/bricks/Greybricks.png";     

        std::string tank_texture_path = "assets/textures/free-2d-battle-tank-game-assets/PNG/Hulls_Color_A/Hull_01.png";
    };

    struct Config {
        int screen_width=600;
        int screen_height=600;

        std::string font_path="/usr/share/fonts/TTF/Hack-Bold.ttf";
        int font_size=24;

        TexturePackPathes texture_pack_pathes;
    };

    struct TexturePack {
        raii::SDL_Texture wall_tile_texture=nullptr;
        raii::SDL_Texture empty_tile_texture=nullptr;
        raii::SDL_Texture tank_texture=nullptr;

        void load(SDL_Renderer *renderer, const TexturePackPathes &pathes) {
            SDL_Texture *texture=nullptr;
            requireSDLCondition(texture=load_texture(renderer, pathes.wall_texture_path));
            wall_tile_texture.reset(texture);

            requireSDLCondition(texture=load_texture(renderer, pathes.empty_texture_path));
            empty_tile_texture.reset(texture);

            requireSDLCondition(texture=load_texture(renderer, pathes.tank_texture_path));
            tank_texture.reset(texture);
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
                int w = world.map().tile_sz;
                int h = world.map().tile_sz;
                api::Tile tile = grid[y][x];
                switch (tile.type) {
                    case api::Tile::Type::WALL:  draw_texture(renderer_.get(), texture_pack_.wall_tile_texture.get(), x * w, y * h, w, h); break;
                    case api::Tile::Type::EMPTY: draw_texture(renderer_.get(), texture_pack_.empty_tile_texture.get(), x * w, y * h, w, h); break;
                    default: 
                    assert(false && "invalid tile type");
                }
            }
        }

        for (auto &tank : world.tanks()) {
            SDL_Rect tank_rect = { 
                static_cast<int>(tank.pos.x), 
                static_cast<int>(tank.pos.y), 
                static_cast<int>(tank.hitbox_sz.x), 
                static_cast<int>(tank.hitbox_sz.y) 
            };

            float angle = convert_dir_to_angle(tank.dir);
            SDL_Point center = { 
                static_cast<int>(tank.hitbox_sz.x / 2), 
                static_cast<int>(tank.hitbox_sz.y / 2)
            }; 

            SDL_RenderCopyEx(renderer_.get(),
                            texture_pack_.tank_texture.get(),
                            nullptr,       
                            &tank_rect,
                            angle,         
                            &center,       
                            SDL_FLIP_NONE);


            SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 0, 255); 
            SDL_RenderDrawRect(renderer_.get(), &tank_rect);
        }
        SDL_RenderPresent(renderer_.get());
    }

    void show() {
    }

private:
    static float convert_dir_to_angle(const api::Dir dir) {
        switch (dir) {
            case api::Dir::UP: return 0;
            case api::Dir::RIGHT: return 90;
            case api::Dir::DOWN: return 180;
            case api::Dir::LEFT: return 270;
            default: assert(false && "unknown api::Dir: " && (int) dir); return 0;
        }
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

    void update() { 
        graphics_.render(world_); 
    }

    api::Input sendInput() const override {
        return {};
    }

    // receive game state from server
    void receive(api::GameState &state) override {
        world_.apply_game_state(state);
    }

    bool connect(const std::string &ip, int port) override {
        return true;
    }

    bool isConnected() const override {
        return true;
    }
};


} // namespace client
