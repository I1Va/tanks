#include <vector>

#include "client.hpp"
#include "server.hpp"

ServerWorld create_world() {
    const size_t height = 50;
    const size_t width = 50;
    GameMap map(height, width);

    for (size_t i = 0; i < width; i++) map.grid[0][i].type = Tile::Type::WALL;
    for (size_t i = 0; i < width; i++) map.grid[height - 1][i].type = Tile::Type::WALL;
    for (size_t i = 0; i < height; i++) map.grid[i][0].type = Tile::Type::WALL;
    for (size_t i = 0; i < height; i++) map.grid[i][width - 1].type = Tile::Type::WALL;

    ServerWorld world(map);
    return world;
}


int main() {
    ServerWorld world = create_world();

    client::TanksGame::Config config;
    client::TanksGame client_game(config);
    Server server;

    // server.add_client([&client](const WorldSnapshot &snapshot){ client.receive_snapshot(snapshot); });
    // while (true) {
    //     server.server_step(world);
    //     client.draw();
    // }

    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }
        client_game.show();   
    }
}