#include <vector>
#include <chrono>

#include "client.hpp"
#include "server.hpp"

const char MAP_PATH[] = "assets/map/map";
const size_t TILE_SZ = 60;



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
    client::Client client_game(config);
    server::Server server(map);

    server.add_client(&client_game);
    api::TankId tank_id = server.spawn_tank_in_tile({5, 5});


    const int FPS = 1;
    const int frameDelay = 1000 / FPS; // milliseconds

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

        api::TankInfo tank; 
        int res = server.get_tank_info(tank_id, tank); assert(res == 0);

        server.tank_rotate(tank_id, api::RotationDir::RIGHT);
        server.tank_move_torward(tank_id);
        server.turret_fire(tank_id);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        
    }
}