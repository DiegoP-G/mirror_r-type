#include "GameMediator.hpp"
#include "../Network/NetworkManager.hpp"
#include <iostream>
#include <memory>

std::string generateLobbyUid()
{
    static int counter = 0;
    return "lobby_" + std::to_string(++counter);
}

GameMediator::GameMediator() : _networkManager(*new NetworkManager(*this)), _lobbyManager(*new LobbyManager(*this))
{

    _mediatorMap = {
        {GameMediatorEvent::SetupNetwork, [this](const std::string &, const std::string &, int) -> void {}},

        {GameMediatorEvent::TickNetwork,
         [this](const std::string &, const std::string &, int) -> void { _networkManager.updateAllPoll(); }},

        {GameMediatorEvent::AddPlayer,
         [this](const std::string &data, const std::string &lobbyUid, int clientFd) -> void {
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobby(lobbyUid);
             if (!lobby)
             {
                 std::cout << "[AddPlayer] Lobby not found, creating: " << lobbyUid << "\n";
                 lobby = _lobbyManager.createLobby(lobbyUid);
             }

             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             int id = deserializeInt(data);
             rtype->createPlayer(data);
             lobby->addPlayer(id);
         }},

        {GameMediatorEvent::EntityCreated,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_ENTITY_CREATE);
         }},

        {GameMediatorEvent::EntityDestroyed,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_ENTITY_DESTROY);
         }},

        {GameMediatorEvent::MovementUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_MOVEMENT_UPDATE);
         }},

        {GameMediatorEvent::HealthUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyUDP(lobby, data, OPCODE_HEALTH_UPDATE);
         }},

        {GameMediatorEvent::PlayerInput,
         [this](const std::string &data, const std::string &lobbyUid, int clientFd) -> void {
             InputComponent inputComp;
             int playerId = deserializePlayerInput(data, inputComp);
             std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(playerId);
             if (!lobby)
             {
                 std::cerr << "[PlayerInput] Player not in a lobby.\n";
                 return;
             }
             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             rtype->handlePlayerInput(data);
         }},

        {GameMediatorEvent::LobbyInfoUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_LOBBY_INFO);
         }},

        {GameMediatorEvent::UpdateWave,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_UPDATE_WAVE);
         }},

        {GameMediatorEvent::UpdateScore,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_UPDATE_SCORE);
         }},

        {GameMediatorEvent::GameOver,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_GAME_OVER);
         }},

        {GameMediatorEvent::PlayerDead,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_PLAYER_DEAD);
         }},

        {GameMediatorEvent::GameStateUpdate,
         [this](const std::string &data, const std::string &lobbyUid, int) -> void {
             std::cout << "updating states" << std::endl;
             auto lobby = _lobbyManager.getLobby(lobbyUid);
             if (lobby)
                 _networkManager.sendDataToLobbyTCP(lobby, data, OPCODE_GAME_STATE_UPDATE);
         }},

        {GameMediatorEvent::CreateLobby,
         [this](const std::string &data, const std::string &, int) -> void { _lobbyManager.createLobby(data); }},

        {GameMediatorEvent::JoinLobby,
         [this](const std::string &data, const std::string &, int clientFd) -> void {
             auto lobby = _lobbyManager.getLobby(data);
             if (!lobby)
             {
                 std::cout << "[JoinLobby] Client " << clientFd << " joining " << data << std::endl;
             }
             std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
             rtype->createPlayer(data);
             lobby->addPlayer(clientFd);
             _networkManager.sendAllEntitiesToClient(clientFd);
         }},
    };
}

void GameMediator::notify(const int &event, const std::string &data, const std::string &lobbyUid, int clientFd)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        it->second(data, lobbyUid, clientFd);
    }
    else
    {
        std::cerr << "[ERROR] Unknown event received by mediator: " << event << std::endl;
    }
}

std::vector<std::string> GameMediator::getAllActiveEntitiesFromLobby(int clientFd)
{
    std::shared_ptr<Lobby> lobby = _lobbyManager.getLobbyOfPlayer(clientFd);
    if (!lobby)
        return {};
    std::unique_ptr<RTypeServer> &rtype = lobby->getRTypeServer();
    return rtype->serializeAllActiveEntities();
}
