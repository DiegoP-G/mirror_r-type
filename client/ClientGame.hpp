#pragma once

#include "Network/NetworkManager.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include "NetworkECSMediator.hpp"
#include "RType.hpp"
#include <atomic>
#include <thread>

class ClientGame
{
  private:
    NetworkECSMediator _med;
    Sender _sender;
    Receiver _receiver;
    NetworkManager _networkManager;
    RTypeGame _graphic;
    std::thread _networkThread;
    std::atomic<bool> _running;

    void networkLoop();

  public:
    ClientGame();
    ~ClientGame();

    bool init(const char *serverIp = "127.0.0.1", int port = 8080);

    void start();
    void stop();
};
