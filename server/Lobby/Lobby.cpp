#include "Lobby.hpp"
#include "../Game/RTypeServer.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

Lobby::Lobby(const std::string &uid, GameMediator &gameMediator)
    : _uid(uid), _rtypeGame(std::make_unique<RTypeServer>(gameMediator)), _running(false), _deltaTime(0.0f)
{
}

Lobby::~Lobby()
{
    stop();
}

void Lobby::run()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_running)
        {
            std::cout << "[Lobby] Lobby " << _uid << " is already running.\n";
            return;
        }
        _running = true;
        std::cout << "[Lobby] Starting game loop for lobby " << _uid << std::endl;
    }

    _rtypeGame->init();
    auto previousTime = std::chrono::high_resolution_clock::now();
    constexpr float targetFrameTime = 1.0f / 60.0f; // 60 ticks per second

    while (_running)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - previousTime;
        previousTime = currentTime;

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _deltaTime = elapsed.count();
        }

        update();

        float sleepTime = targetFrameTime - _deltaTime;
        if (sleepTime > 0.0f)
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
    }

    std::cout << "[Lobby] Exiting game loop for lobby " << _uid << std::endl;
}

void Lobby::update()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_running)
        return;

    _rtypeGame->update(_deltaTime);
}

void Lobby::stop()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_running)
            return;
        _running = false;
    }
    std::cout << "[Lobby] Stopping RTypeServer for lobby " << _uid << std::endl;
}

void Lobby::addPlayer(int fd)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _connectedClients.insert(fd);
    std::cout << "[Lobby] Player " << fd << " joined lobby " << _uid << std::endl;
}

void Lobby::removePlayer(int fd)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _connectedClients.erase(fd);
    std::cout << "[Lobby] Player " << fd << " left lobby " << _uid << std::endl;
}

std::string Lobby::getUid() const
{
    return _uid;
}

std::vector<int> Lobby::getPlayers()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return std::vector<int>(_connectedClients.begin(), _connectedClients.end());
}

bool Lobby::isRunning() const
{
    return _running.load();
}

float Lobby::getDeltaTime()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _deltaTime;
}

std::unique_ptr<RTypeServer> &Lobby::getRTypeServer()
{
    return _rtypeGame;
}