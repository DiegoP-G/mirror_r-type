#pragma once

#include "Network/NetworkManager.hpp"
#include "NetworkECSMediator.hpp"
#include <atomic>
#include <thread>

class ClientGame
{
  private:
    NetworkManager _networkManager;
    std::thread _networkThread;
    std::atomic<bool> _running;

    void networkLoop();

  public:
    ClientGame(NetworkECSMediator &med);
    ~ClientGame();

    bool init(const char *serverIp = "127.0.0.1", int port = 8080);

    void start();
    void stop();
};
