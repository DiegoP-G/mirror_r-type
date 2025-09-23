#pragma once

#include "Network/NetworkManager.hpp"
#include <thread>
#include <atomic>

class ClientGame {
private:
    NetworkManager _networkManager;
    std::thread _networkThread;
    std::atomic<bool> _running;

    void networkLoop();

public:
    ClientGame();
    ~ClientGame();

    bool init(const char* serverIp = "127.0.0.1", int port = 8080);

    void start();
    void stop();
};
