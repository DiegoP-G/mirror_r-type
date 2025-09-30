#include "ClientGame.hpp"
#include "../ecs/GraphicsManager.hpp"
#include "Network/NetworkManager.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include "NetworkECSMediator.hpp"
#include <iostream>

ClientGame::ClientGame()
    : _med(), _sender(Sender(_med)), _receiver(Receiver(_med)), _game(RTypeGame()),
      _networkManager(NetworkManager(_med, _sender, _receiver)), _running(false)
{
    _med.setSender(&_sender);
    _med.setReceiver(&_receiver);
    _med.setRTypeGame(&_game);
}

ClientGame::~ClientGame()
{
    stop();
}

bool ClientGame::init(const char *serverIp, int port)
{
    if (!_networkManager.setup(serverIp, port))
        return false;
    if (!_game.init(_med))
        return false;
    return true;
}

void ClientGame::start()
{
    if (_running)
        return;

    _running = true;
    _networkThread = std::thread(&ClientGame::networkLoop, this);

    std::cout << "ClientGame started (network running in background)" << std::endl;

    std::cout << "entrypoint" << std::endl;
    _game.run();
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
