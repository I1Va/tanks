#include <vector>
#include <chrono>

#include "client.hpp"
#include "server.hpp"

api::GameMap create_game_map() {
    const size_t height = 10;
    const size_t width = 10;
    const size_t tile_sz = 60;
    api::GameMap map(height, width, tile_sz);
    // for (size_t x = 0; x < width; x++) {
    //     for (size_t y = 0; y < height; y++) {
    //         map.grid[y][x].type = api::Tile::Type::EMPTY;
    //     }
    // }

    for (size_t x = 0; x < width; x++) {
        map.grid[0][x].type = api::Tile::Type::WALL;
    }

    // bottom row
    for (size_t x = 0; x < width; x++) {
        map.grid[height - 1][x].type = api::Tile::Type::WALL;
    }

    // left column (excluding corners if already set is optional)
    for (size_t y = 0; y < height; y++) {
        map.grid[y][0].type = api::Tile::Type::WALL;
    }

    // right column
    for (size_t y = 0; y < height; y++) {
        map.grid[y][width - 1].type = api::Tile::Type::WALL;
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

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        
    }
}