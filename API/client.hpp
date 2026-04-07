#pragma once


namespace api
{

struct GameState;

struct Input {};

class IClient {
public:
    virtual ~IClient() = default;

    // connect to server
    virtual bool connect(const std::string &ip, int port) = 0;

    // receive game state from server
    virtual void receive(GameState &state) = 0;

    // optional: check connection
    virtual bool isConnected() const = 0;
};

}