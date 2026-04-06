#include <vector>


#include "client.hpp"
#include "server.hpp"


int main() {
    const size_t height = 100;
    const size_t widhth = 100;
    ServerWorld world(height, widhth);

    Client client;
    Server server;

    server.add_client([&client](const WorldSnapshot &snapshot){ client.receive_snapshot(snapshot); });
    while (true) {
        server.server_step(world);
        client.draw();
    }
}