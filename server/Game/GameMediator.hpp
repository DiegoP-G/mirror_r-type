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
    UpdateEntities = OPCODE_UPDATE_ENTITIES,
    UpdateEntitiesZlib = OPCODE_UPDATE_ENTITIES_ZLIB,
    PlayerInput = OPCODE_PLAYER_INPUT,
    EntitiesCreated = OPCODE_ENTITY_CREATE,
    EntityDestroyed = OPCODE_ENTITY_DESTROY,
    GameOver = OPCODE_GAME_OVER,
    PlayerDead = OPCODE_PLAYER_DEAD,
    UpdateWave = OPCODE_UPDATE_WAVE,
    UpdateScore = OPCODE_UPDATE_SCORE,
    CreateLobby = OPCODE_CREATE_LOBBY,
    JoinLobby = OPCODE_JOIN_LOBBY,
    JoinGameLobby = OPCODE_GAME_LOBBY,
    LoginReqest = 0x80,
    LoginResponse = 0x81,
    SigninRequest = 0x82,
    SigninResponse = 0x83,
    ServerPubKey = 0x90,
    ClientIVKey = 0x92,
    VoiceComming = 0x75,
    PlayerBonus = 0x93,
    NewWave = 0x94,
    Explosion = 0x95
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

    std::string getAllActiveEntitiesFromLobby(int fd);

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

    NetworkManager &getNetworkManager()
    {
        return _networkManager;
    };

    LobbyManager &getLobbyManager()
    {
        return _lobbyManager;
    };
};
