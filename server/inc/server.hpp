#pragma once

#include <chrono>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>

#include "server_api/api.hpp"


class ServerWorld { // GameWorld logic
    GameMap map_;
    uint64_t tick_;
public:
    ServerWorld(const size_t height, const size_t width): map_(height, width) {}

    void simulate_step(float dt) {
        ++tick_;
    }
    
    WorldSnapshot create_snapshot() const {
        return WorldSnapshot{map_, tick_};
    }
};

class Server { // API + client-server connection logic     
    std::vector<std::function<void(const WorldSnapshot &)>> clients_;
public:
    void send_snapshot_to_clients(const WorldSnapshot &snap) {
        for (auto client : clients_) {
            client(snap);
        }
    }

    void run_server(ServerWorld &world) {
        const float TICK_RATE = 30.0f;
        const float dt = 1.0f / TICK_RATE;

        while (true) {
            auto start = std::chrono::steady_clock::now();

            world.simulate_step(dt);

            // 3) generate and send snapshot to all connected clients
            WorldSnapshot snap = world.create_snapshot();
            send_snapshot_to_clients(snap);

            // 4) sleep until next tick
            std::this_thread::sleep_until(start + std::chrono::duration<float>(dt));
        }
    }

    void server_step(ServerWorld &world) {
        const float TICK_RATE = 30.0f;
        const float dt = 1.0f / TICK_RATE;
        auto start = std::chrono::steady_clock::now();

        world.simulate_step(dt);

        // 3) generate and send snapshot to all connected clients
        WorldSnapshot snap = world.create_snapshot();
        send_snapshot_to_clients(snap);

        // 4) sleep until next tick
        std::this_thread::sleep_until(start + std::chrono::duration<float>(dt));
    }

    void add_client(std::function<void(const WorldSnapshot &)> client) {
        clients_.push_back(client);
    }
    
};


