#pragma once

#include "Lobby.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

class GameMediator;

class LobbyManager
{
  public:
    LobbyManager(GameMediator &med) : _med(med){};
    ~LobbyManager();

    LobbyManager(const LobbyManager &) = delete;
    LobbyManager &operator=(const LobbyManager &) = delete;

    std::shared_ptr<Lobby> createLobby(const std::string &uid);
    std::shared_ptr<Lobby> getLobby(const std::string &uid);
    std::shared_ptr<Lobby> getLobbyOfPlayer(int clientId);
    void removeLobby(const std::string &uid);

    void listLobbies() const;

  private:
    GameMediator &_med;
    std::unordered_map<std::string, std::shared_ptr<Lobby>> _lobbies;
    std::unordered_map<std::string, std::thread> _threads;
    mutable std::mutex _mutex;
};
