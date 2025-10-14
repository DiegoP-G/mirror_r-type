#pragma once
#include "../Lobby/LobbyManager.hpp"
#include "../Mediator/IMediator.hpp"
#include "../Network/NetworkManager.hpp"
#include "RTypeServer.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

enum GameMediatorEvent
{
    TickNetwork,
    TickLogic,
    AddPlayer,
    SetupNetwork,
    InitECS,
    LobbyInfoUpdate,
    GameStateUpdate,
    PlayerDisconnected,
    UpdateEntities = 0x21,
    UpdatePlayers = 0x23,
    UpdateEnemies = 0x24,
    UpdateProjectiles = 0x25,
    PlayerInput = 0x26,
    EntityCreated = 0x27,
    EntityDestroyed = 0x28,
    MovementUpdate = 0x29,
    GameOver = 0x32,
    PlayerDead = 0x33,
    HealthUpdate = 0x40,
    UpdateWave = 0x50,
    UpdateScore = 0x51,
    CreateLobby = 0x70,
    JoinLobby = 0x71
};

class NetworkManager;
class RTypeServer;

class GameMediator : public IMediator
{
  private:
    NetworkManager &_networkManager;
    LobbyManager &_lobbyManager;

    std::unordered_map<GameMediatorEvent, std::function<void(const std::string &, const std::string &, int)>>
        _mediatorMap;

  public:
    GameMediator();
    ~GameMediator() = default;

    void notify(const int &event, const std::string &data = "", const std::string &lobbyUID = "",
                int clientFd = -1) override;
    inline void sendMessageToAll(const std::string &data) override
    {
        return;
    };

    std::vector<std::string> getAllActiveEntitiesFromLobby(int fd);

    std::string toString(GameMediatorEvent event)
    {
        switch (event)
        {
        case TickNetwork:
            return "Ticked Network";
        case TickLogic:
            return "Ticked Logic";
        case SetupNetwork:
            return "Setup Network";
        default:
            return "UnknownEvent";
        }
    }
};
