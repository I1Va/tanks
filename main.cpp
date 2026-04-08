#include <vector>
#include <chrono>

#include "client.hpp"
#include "server.hpp"

#include "script1.hpp"
#include "script2.hpp"
#include "script3.hpp"

const char MAP_PATH[] = "assets/map/map";
const size_t TILE_SZ = 50;

api::GameMap create_game_map() {
    std::ifstream file(MAP_PATH);
    if (!file) throw std::runtime_error("failed to load map : `" + std::string(MAP_PATH) + "`");

    int height = 0;
    int width = 0;
    file >> height >> width;
    
    api::GameMap map(height, width, TILE_SZ);
    std::string key;

    std::map<std::string, api::Tile::Type> tile_map;
    tile_map["Wall"] = api::Tile::Type::Wall;
    tile_map["Floor"] = api::Tile::Type::Floor;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            file >> key;
            if (!tile_map.contains(key)) key = "Floor";
            map.grid[y][x].type = tile_map[key];            
        }
    }
    
    return map;
}

int main() {
    api::GameMap map = create_game_map();

    client::Client::Config config;
    config.gfx_config.screen_height = map.grid.size() * map.tile_sz;
    config.gfx_config.screen_width = map.grid[0].size() * map.tile_sz;

    client::Client client_game(config);
    server::Server server(map);

    server.add_client(&client_game);

    npc1_init(server, {3, 3});
    npc2_init(server, {9, 15});
    npc3_init(server, {15, 5});
    server.add_npc_script(npc1_step);
    server.add_npc_script(npc2_step);
    server.add_npc_script(npc3_step);

    const int FPS = 10;
    const int frameDelay = 1000 / FPS;

    Uint32 frameStart;
    int frameTime;
    bool running = true;
    SDL_Event e;
    while (running) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        server.update();
        client_game.update();

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        
    }
}