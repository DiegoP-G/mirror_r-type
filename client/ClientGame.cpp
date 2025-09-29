#include "ClientGame.hpp"
#include "Network/NetworkManager.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include "NetworkECSMediator.hpp"
#include <iostream>

ClientGame::ClientGame()
    : _med(), _sender(Sender(_med)), _receiver(Receiver(_med)),
      _networkManager(NetworkManager(_med, _sender, _receiver)), _running(false)
{
    _med.setSender(&_sender);
    _med.setReceiver(&_receiver);
}

ClientGame::~ClientGame()
{
    stop();
}

bool ClientGame::init(const char *serverIp, int port)
{
    return _networkManager.setup(serverIp, port);
}

void ClientGame::start()
{
    if (_running)
        return;

    _running = true;
    _networkThread = std::thread(&ClientGame::networkLoop, this);

    std::cout << "ClientGame started (network running in background)" << std::endl;

    _graphic.init();
    _graphic.run();
}

void ClientGame::stop()
{
    if (_running)
    {
        _running = false;
        if (_networkThread.joinable())
        {
            _networkThread.join();
        }
        std::cout << "ClientGame stopped" << std::endl;
    }
}

void ClientGame::networkLoop()
{
    // while (_running) {
    _networkManager.loop();
    // }
}
