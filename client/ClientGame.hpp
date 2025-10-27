#pragma once

#include "../ecs/GraphicsManager.hpp"
#include "Network/NetworkManager.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include "NetworkECSMediator.hpp"
#include "VoiceManager.hpp"
#include <atomic>
#include <thread>

class ClientGame
{
  private:
    NetworkECSMediator _med;
    Sender _sender;
    Receiver _receiver;
    VoiceManager _voiceManager;

    RTypeGame _game;
    NetworkManager _networkManager;
    std::thread _networkThread;
    std::atomic<bool> _running;

    void networkLoop();

  public:
    ClientGame();
    ~ClientGame();

    bool init(const char *serverIp = "127.0.0.1", int port = 8080);
    void startServer(const char *serverIp);

    void start();
    void stop();
};
