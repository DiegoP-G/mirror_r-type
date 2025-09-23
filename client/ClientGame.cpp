#include "ClientGame.hpp"
#include <iostream>

ClientGame::ClientGame() : _running(false) {}

ClientGame::~ClientGame() {
    stop();
}

bool ClientGame::init(const char* serverIp, int port) {
    return _networkManager.setup(serverIp, port);
}

void ClientGame::start() {
    if (_running) return;

    _running = true;
    _networkThread = std::thread(&ClientGame::networkLoop, this);

    std::cout << "ClientGame started (network running in background)" << std::endl;
}

void ClientGame::stop() {
    if (_running) {
        _running = false;
        if (_networkThread.joinable()) {
            _networkThread.join();
        }
        std::cout << "ClientGame stopped" << std::endl;
    }
}

void ClientGame::networkLoop() {
    // while (_running) {
    _networkManager.loop(); 
    // }
}
