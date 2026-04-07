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

    raii::SDL_Texture body=nullptr;
    Vec2f body_center={};
    Vec2f turret_slot_center={};

    raii::SDL_Texture turret=nullptr;
    Vec2f turret_center={};

    raii::SDL_Texture trackA=nullptr;
    raii::SDL_Texture trackB=nullptr;

    Vec2f parse_turret_center(std::ifstream &file) {
        std::string key;
        Vec2f center;

        while (file >> key) {
            if (key == "turret_center") {
                file >> center.x >> center.y;
                return center;
            }
        }
    
        throw std::runtime_error("turret_center was not found in render info");
    }

    void parse_render_info(const std::string &path) {
        std::ifstream file(path + "/" + RENDER_INFO_PATH);
        if (!file) throw std::runtime_error("failed to load render_info : `" + RENDER_INFO_PATH + "`");

        std::string key;
        Vec2f center;

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
                switch (tile.type) {
                    case api::Tile::Type::WALL:  draw_texture(renderer_.get(), tile_texture_pack.wall.get(),  x * w, y * h, w, h); break;
                    case api::Tile::Type::EMPTY: draw_texture(renderer_.get(), tile_texture_pack.floor.get(), x * w, y * h, w, h); break;
                    default: 
                    assert(false && "invalid tile type");
                }
            }
        }
    }

    void render_tank(const Tank &tank) {
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

        int w, h;
        SDL_QueryTexture(tank_texture_pack.body.get(), nullptr, nullptr, &w, &h);

        SDL_RenderCopyEx(renderer_.get(),
                        tank_texture_pack.body.get(),
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