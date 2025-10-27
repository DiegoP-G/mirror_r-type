#include "ClientGame.hpp"
#include "../ecs/GraphicsManager.hpp"
#include "Network/NetworkManager.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include "NetworkECSMediator.hpp"
#include "transferData/hashUtils.hpp"
#include <iostream>

ClientGame::ClientGame()
    : _sender(Sender(_med)), _receiver(Receiver(_med)), _game(RTypeGame(_med)),
      _networkManager(NetworkManager(_med, _sender, _receiver)), _med(_networkManager), _running(false)
{
    _med.setSender(&_sender);
    _med.setReceiver(&_receiver);
    _med.setRTypeGame(&_game);
}

ClientGame::~ClientGame()
{
    stop();
}

void ClientGame::startServer(const char *serverIp)
{
    std::cout << "[Client] Attempting to connect to server at " << serverIp << std::endl;
    if (!_networkManager.setup(serverIp, 8081))
    {
        std::cout << "failed to start network" << std::endl;
        return;
    }
    _networkThread = std::thread(&ClientGame::networkLoop, this);
    _game.setCurrentState(GameState::MENULOGIN);

    std::cout << "[Client] Connected successfully, showing lobby menu" << std::endl;
}

bool ClientGame::init(const char *serverIp, int port)
{
    // if (!_networkManager.setup(serverIp, port))
    //     return false;
    if (!_game.init(_med, [this](const char *msg) { this->startServer(msg); }))
        return false;
    return true;
}

void ClientGame::start()
{
    if (_running)
        return;

    _running = true;
    // _networkThread = std::thread(&ClientGame::networkLoop, this);

    std::cout << "ClientGame started (network running in background)" << std::endl;

    _game.run();
    stop();
}

void ClientGame::stop()
{
    if (_running)
    {
        _running = false;
        _networkManager.stop(); // Arrêter la boucle réseau

        if (_networkThread.joinable())
        {
            _networkThread.join();
        }
        std::cout << "ClientGame stopped" << std::endl;
    }
}

void ClientGame::networkLoop()
{
    _networkManager.loop();
}
