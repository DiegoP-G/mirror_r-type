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
        std::cerr << "[LobbyManager] Cannot create lobby: " << uid << " already exists.\n";
        return nullptr;
    }

    std::cout << "UID IN MANAGER: " << uid << std::endl;
    auto lobby = std::make_shared<Lobby>(uid, _med);
    _lobbies[uid] = lobby;

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
    std::unique_lock<std::mutex> lock(_mutex);

    auto it = _lobbies.find(uid);
    if (it == _lobbies.end())
        return;

    // Grab thread object and lobby ptr safely before releasing the lock
    auto thread_it = _threads.find(uid);
    auto lobby = it->second;

    _lobbies.erase(it);
    lock.unlock(); // 🔓 release before stopping or joining

    lobby->stop();

    if (thread_it != _threads.end() && thread_it->second.joinable())
    {
        thread_it->second.join();
        std::lock_guard<std::mutex> relock(_mutex);
        _threads.erase(thread_it);
    }

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
