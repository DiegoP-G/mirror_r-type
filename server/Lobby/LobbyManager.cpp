#include "LobbyManager.hpp"
#include "../Game/GameMediator.hpp"
#include <iostream>

LobbyManager::~LobbyManager()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &[uid, lobby] : _lobbies)
    {
        lobby->stop();
    }
    for (auto &[uid, thread] : _threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

std::shared_ptr<Lobby> LobbyManager::createLobby(const std::string &uid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_lobbies.find(uid) != _lobbies.end())
    {
        std::cerr << "[LobbyManager] Lobby " << uid << " already exists.\n";
        return _lobbies[uid];
    }

    std::cout << "UID IN MANAGER" << uid << std::endl;
    auto lobby = std::make_shared<Lobby>(uid, _med);
    _lobbies[uid] = lobby;

    // Run the lobby in its own thread
    _threads[uid] = std::thread([lobby]() { lobby->run(); });

    std::cout << "[LobbyManager] Created and launched lobby " << uid << std::endl;
    return lobby;
}

std::shared_ptr<Lobby> LobbyManager::getLobby(const std::string &uid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _lobbies.find(uid);
    if (it != _lobbies.end())
    {
        return it->second;
    }
    return nullptr;
}

void LobbyManager::removeLobby(const std::string &uid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _lobbies.find(uid);
    if (it == _lobbies.end())
        return;

    it->second->stop();

    if (_threads.find(uid) != _threads.end() && _threads[uid].joinable())
    {
        _threads[uid].join();
        _threads.erase(uid);
    }

    _lobbies.erase(uid);
    std::cout << "[LobbyManager] Removed lobby " << uid << std::endl;
}

void LobbyManager::listLobbies() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << "=== Active Lobbies ===\n";
    for (const auto &[uid, lobby] : _lobbies)
    {
        std::cout << "- " << uid << " (" << lobby->getPlayers().size() << " players)\n";
    }
}

std::shared_ptr<Lobby> LobbyManager::getLobbyOfPlayer(int clientId)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (const auto &[uid, lobby] : _lobbies)
    {
        auto players = lobby->getPlayers(); // already thread-safe

        if (std::find(players.begin(), players.end(), clientId) != players.end())
        {
            return lobby;
        }
    }

    return nullptr;
}
