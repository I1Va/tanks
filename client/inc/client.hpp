#pragma once

#include <iostream>

#include "server_api/api.hpp"

class Client {
    // GameMap map_;
    uint64_t tick_;

public:
    // Client(const size_t height, const size_t width): map_(height, width) {}

    void receive_snapshot(const WorldSnapshot &snapshot) {
        // map_ = snapshot.map;
        tick_ = snapshot.tick;

        redraw();
    }

    void draw() {
        std::cout << "client tick: " << tick_ << "\n";
    }

private:
    void redraw() {

    }

};
