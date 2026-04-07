#pragma once

#include <fstream>

#include <SDL2/SDL_image.h>
#include "SDLRAII.hpp"
#include "SDL_utility.hpp"
#include "err_proc.hpp"
#include "client_world.hpp"

#include "API/client.hpp"
#include "API/server.hpp"

namespace client 
{

struct TTF_SDL_Init_Guard {
    TTF_SDL_Init_Guard() { if (TTF_Init()!=0) throw TTFException(); }
    ~TTF_SDL_Init_Guard() { TTF_Quit(); }
};

struct SDL_Init_Guard {
    SDL_Init_Guard() { if (SDL_Init(SDL_INIT_VIDEO)!=0) throw SDLException(); }
    ~SDL_Init_Guard() { SDL_Quit(); }
};

struct TexturePackPathes {
    std::string walls_texture_pack="assets/texture_packs/tiles1";  
    std::string tank_texture_pack="assets/texture_packs/tank1";
};

struct TileTexturePack {
    static constexpr std::string WALL_PATH = "wall.png";
    static constexpr std::string FLOOR_PATH = "floor.png";
  
    raii::SDL_Texture wall=nullptr;
    raii::SDL_Texture floor=nullptr;

    void load(SDL_Renderer *renderer, const std::string &path) {
        SDL_Texture *texture=nullptr;
        requireSDLCondition(texture=load_texture(renderer, path + "/" + WALL_PATH));
        wall.reset(texture);

        requireSDLCondition(texture=load_texture(renderer, path + "/" + FLOOR_PATH));
        floor.reset(texture);
    }
};

struct TankTexturePack {
    static constexpr std::string BODY_PATH = "body.png";
    static constexpr std::string TURRET_PATH = "turret.png";
    static constexpr std::string TRACK_A_PATH = "trackA.png";
    static constexpr std::string TRACK_B_PATH = "trackB.png";
    static constexpr std::string RENDER_INFO_PATH = "render_info";

    raii::SDL_Texture tank_base=nullptr;

    raii::SDL_Texture body=nullptr;
    SDL_Point body_center={};
    SDL_Point turret_slot_center={};

    raii::SDL_Texture turret=nullptr;
    SDL_Point turret_center={};

    raii::SDL_Texture trackA=nullptr;
    raii::SDL_Texture trackB=nullptr;

    void parse_render_info(const std::string &path) {
        std::ifstream file(path + "/" + RENDER_INFO_PATH);
        if (!file) throw std::runtime_error("failed to load render_info : `" + RENDER_INFO_PATH + "`");

        std::string key;
        SDL_Point center;

        bool body_center_parsed = false;
        bool turret_slot_center_parsed = false;
        bool turret_center_parsed = false;
        while (file >> key && file >> center.x >> center.y) {
            if (key == "body_center") { body_center = center; body_center_parsed = true; continue; }
            if (key == "turret_slot_center") { turret_slot_center = center; turret_slot_center_parsed = true; continue; }
            if (key == "turret_center") { turret_center = center; turret_center_parsed = true; continue; }
        }
        if (!body_center_parsed) std::cerr << "render info : " << "body_center" << "was not parsed\n";
        if (!turret_slot_center_parsed) std::cerr << "render info : " << "turret_slot_center" << "was not parsed\n";
        if (!turret_center_parsed) std::cerr << "render info : " << "turret_center" << "was not parsed\n";
    }

    void load(SDL_Renderer *renderer, const std::string &path) {
        SDL_Texture *texture=nullptr;
        
        requireSDLCondition(texture=load_texture(renderer, path + "/" + BODY_PATH));
        body.reset(texture);

        requireSDLCondition(texture=load_texture(renderer, path + "/" + TURRET_PATH));
        turret.reset(texture);

        requireSDLCondition(texture=load_texture(renderer, path + "/" + TRACK_A_PATH));
        trackA.reset(texture);

        requireSDLCondition(texture=load_texture(renderer, path + "/" + TRACK_B_PATH));
        trackB.reset(texture);

        parse_render_info(path);

        SDL_Point tank_texture_size;
        SDL_QueryTexture(body.get(), nullptr, nullptr, &tank_texture_size.x, &tank_texture_size.y);

        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            tank_texture_size.x,
            tank_texture_size.y
        );
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        tank_base.reset(texture);
    }
};

class Graphics {
public:
    struct Config {
        int screen_width=600;
        int screen_height=600;

        std::string font_path="/usr/share/fonts/TTF/Hack-Bold.ttf";
        int font_size=24;

        TexturePackPathes texture_pack_pathes;
    };

private:
    Config config_;

    SDL_Init_Guard sdl_init;
    TTF_SDL_Init_Guard ttf_init;

    raii::SDL_Window window_=nullptr;
    raii::SDL_Renderer renderer_=nullptr;
    raii::TTF_Font font_=nullptr;

    TankTexturePack tank_texture_pack;
    TileTexturePack tile_texture_pack;

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

        tank_texture_pack.load(renderer_.get(), config.texture_pack_pathes.tank_texture_pack);
        tile_texture_pack.load(renderer_.get(), config.texture_pack_pathes.walls_texture_pack);
    }

    ~Graphics() {}

    void render(const ClientWorld &world) {
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get());

       

        render_map(world);
        
        for (auto &b : world.bullets()) {
            SDL_Rect r = {static_cast<int>(b.pos.x), static_cast<int>(b.pos.y), 4, 4};
            SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 0, 255);
            SDL_RenderFillRect(renderer_.get(), &r);
        }

        for (auto &tank : world.tanks()) {
            render_tank(tank);
        }
        SDL_RenderPresent(renderer_.get());
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

    void render_map(const ClientWorld &world) {
        const std::vector<std::vector<api::Tile>> &grid = world.map().grid;
        for (size_t y = 0; y < grid.size(); y++) {
            for (size_t x = 0; x < world.map().grid[y].size(); x++) {
                int w = world.map().tile_sz;
                int h = world.map().tile_sz;
                api::Tile tile = grid[y][x];
                SDL_Rect dst_rect = {
                    static_cast<int>(x * w),
                    static_cast<int>(y * h),
                    static_cast<int>(w), 
                    static_cast<int>(h)
                };
                switch (tile.type) {
                    case api::Tile::Type::Wall:  SDL_RenderCopy(renderer_.get(), tile_texture_pack.wall.get(), nullptr, &dst_rect); break;
                    case api::Tile::Type::Floor: SDL_RenderCopy(renderer_.get(), tile_texture_pack.floor.get(), nullptr, &dst_rect); break;
                    default: 
                    assert(false && "invalid tile type");
                }
            }
        }
    }

    void render_tank(const Tank &tank) {     
        SDL_SetRenderTarget(renderer_.get(), tank_texture_pack.tank_base.get());
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 0); 
        SDL_RenderClear(renderer_.get());

        // Body
        SDL_Point tank_texture_size;
        SDL_QueryTexture(tank_texture_pack.body.get(), nullptr, nullptr, &tank_texture_size.x, &tank_texture_size.y);
        SDL_Rect tank_texture_rect = {0, 0, tank_texture_size.x, tank_texture_size.y};
        SDL_RenderCopy(renderer_.get(), tank_texture_pack.body.get(), nullptr, &tank_texture_rect);

        // Turret
        SDL_Point turret_texture_size;
        SDL_QueryTexture(tank_texture_pack.turret.get(), nullptr, nullptr, &turret_texture_size.x, &turret_texture_size.y);
        SDL_Rect turret_texture_rect = { 
            tank_texture_pack.turret_slot_center.x - tank_texture_pack.turret_center.x,
            tank_texture_pack.turret_slot_center.y - tank_texture_pack.turret_center.y,
            turret_texture_size.x,
            turret_texture_size.y
        };
        SDL_RenderCopy(renderer_.get(), tank_texture_pack.turret.get(), nullptr, &turret_texture_rect);

        SDL_SetRenderTarget(renderer_.get(), nullptr);

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
                        tank_texture_pack.tank_base.get(),
                        nullptr,
                        &tank_rect,
                        angle,
                        &center,
                        SDL_FLIP_NONE);

        SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 0, 255); 
        SDL_RenderDrawRect(renderer_.get(), &tank_rect);
    }
};

} // namespace client 