#include <vector>

#include "client.hpp"
#include "server.hpp"

server::ServerWorld create_world() {
    const size_t height = 50;
    const size_t width = 50;
    api::GameMap map(height, width);

    for (size_t i = 0; i < width; i++) map.grid[0][i].type = api::Tile::Type::WALL;
    for (size_t i = 0; i < width; i++) map.grid[height - 1][i].type = api::Tile::Type::WALL;
    for (size_t i = 0; i < height; i++) map.grid[i][0].type = api::Tile::Type::WALL;
    for (size_t i = 0; i < height; i++) map.grid[i][width - 1].type = api::Tile::Type::WALL;

    server::ServerWorld world(map);
    return world;
}

int main() {
    server::ServerWorld world = create_world();

    client::Client::Config config;
    client::Client client_game(config);
    server::Server server(world);

    server.add_client(&client_game);

    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }
        server.update();
        client_game.show();   
    }
}