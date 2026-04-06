#include <vector>
#include <chrono>

#include "client.hpp"
#include "server.hpp"

server::ServerWorld create_world() {
    const size_t height = 10;
    const size_t width = 10;
    api::GameMap map(height, width);

    for (size_t x = 0; x < width; x++) {
        map.grid[0][x].type = api::Tile::Type::WALL;
        map.grid[0][x].cord = { int(0), int(x) };
    }

    // bottom row
    for (size_t x = 0; x < width; x++) {
        map.grid[height - 1][x].type = api::Tile::Type::WALL;
        map.grid[height - 1][x].cord = { int(height - 1), int(x) };
    }

    // left column (excluding corners if already set is optional)
    for (size_t y = 0; y < height; y++) {
        map.grid[y][0].type = api::Tile::Type::WALL;
        map.grid[y][0].cord = { int(y), int(0) };
    }

    // right column
    for (size_t y = 0; y < height; y++) {
        map.grid[y][width - 1].type = api::Tile::Type::WALL;
        map.grid[y][width - 1].cord = { int(y), int(width - 1) };
    }
    server::ServerWorld world(map);
    return world;
}

int main() {
    server::ServerWorld world = create_world();

    client::Client::Config config;
    client::Client client_game(config);
    server::Server server(world);

    server.add_client(&client_game);

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
        client_game.show();

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }
}